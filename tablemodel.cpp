#include "tablemodel.h"

TableModel::TableModel()
{
    rootNode = new TreeNode(new Link("", "" ,""));
    rootNode->getLink()->setData(Link::DATA_IS_CONTAINER, QVariant(true));
}

TableModel::~TableModel()
{
    delete rootNode;
}

int TableModel::mapDataToColumn(Link::Data data)
{
    switch (data) {
    case Link::DATA_TITLE: return 0;
    case Link::DATA_PROGRESS: return 1;
    default: return 0;
    }
}

Link::Data TableModel::mapColumnToData(int column)
{
    switch (column) {
    case 0: return Link::DATA_TITLE;
    case 1: return Link::DATA_PROGRESS;
    }
    return Link::DATA_INVALID;
}

Link *TableModel::addLink(QString link, Link *parent)
{
    Link *mediaLink = new Link(link, link, "0%");
    return this->addLink(mediaLink, parent);
}

Link *TableModel::addLink(Link *link, Link *parent)
{
    QModelIndex parentIndex = getIndexForLink(parent, 0);
    if(!parentIndex.isValid())
    {
        beginInsertRows(QModelIndex(), this->rootNode->getChildNodeCount(), this->rootNode->getChildNodeCount());
        this->rootNode->appendChildNode(new TreeNode(link, rootNode));
        endInsertRows();
        return link;
    }

    convertToContainer(parentIndex);

    TreeNode *parentNode = (TreeNode*) parentIndex.internalPointer();
    beginInsertRows(parentIndex, parentNode->getChildNodeCount(), parentNode->getChildNodeCount());
    parentNode->appendChildNode(new TreeNode(link, parentNode));
    endInsertRows();
    return link;
}

void TableModel::deleteIndex(QModelIndex index)
{
    myRemoveRows(index.row(), 1, index.parent());
}

bool TableModel::myRemoveRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row+count-1);

    TreeNode *parentNode = (TreeNode*) parent.internalPointer();
    if(!parent.isValid())
    {
        parentNode = this->rootNode;
    }
    while(count > 0)
    {
        parentNode->deleteChildNode(row);
        count--;
    }

    endRemoveRows();
    return true;
}

bool TableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow)
{
    TreeNode *sourceParentNode = rootNode;
    if(sourceParent.isValid())
    {
       sourceParentNode = (TreeNode*) sourceParent.internalPointer();
    }

    TreeNode *destinationParentNode = rootNode;
    if(destinationParent.isValid())
    {
        destinationParentNode = (TreeNode*) destinationParent.internalPointer();
    }

    if(count <= 0)
    {
        return false;
    }


    convertToContainer(destinationParent);

    int finalDestinationRow = destinationRow==-1?destinationParentNode->getChildNodeCount():destinationRow;
    if(sourceParent == destinationParent && finalDestinationRow >= sourceRow && finalDestinationRow <= sourceRow+count)
    {
        return false;
    }
    beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, finalDestinationRow);
    if(sourceParentNode == destinationParentNode && finalDestinationRow > sourceRow)
    {
        finalDestinationRow -= count;
    }
    for(int i = 0; i < count; i++)
    {
        TreeNode *movingRow = sourceParentNode->getChildNode(sourceRow);
        sourceParentNode->removeChildNode(sourceRow);
        destinationParentNode->insertChildNode(movingRow, finalDestinationRow);
    }
    endMoveRows();
}

void TableModel::convertToContainer(const QModelIndex &index)
{
    TreeNode *indexNode = (TreeNode*) index.internalPointer();
    if(!index.isValid())
    {
        indexNode = this->rootNode;
    }

    if(!indexNode->getLink()->getData(Link::Data::DATA_IS_CONTAINER).toBool())
    {
        Link *newLink = new Link(*(indexNode->getLink()));
        indexNode->getLink()->setData(Link::DATA_IS_CONTAINER, QVariant(true));
        updateLinkProgress(indexNode, "");
        refreshName(indexNode, "Container");
        indexNode->getLink()->setData(Link::DATA_IS_CONTAINER, QVariant(true));
        beginInsertRows(index, indexNode->getChildNodeCount(), indexNode->getChildNodeCount());
        indexNode->appendChildNode(new TreeNode(newLink, indexNode));
        endInsertRows();
    }
}

QModelIndex TableModel::getIndexForLink(Link *link, int column, TreeNode *currentNode) const
{
    if(currentNode == nullptr)
    {
       currentNode = this->rootNode;
    }
    if(link == nullptr)
    {
        return QModelIndex();
    }
    if(currentNode->getLink() == link && currentNode != this->rootNode)
    {
        QModelIndex index = createIndex(currentNode->row(), column, currentNode);
        return index;
    }
    for(int i = 0; i < currentNode->getChildNodeCount(); i++)
    {
        TreeNode *nextNode = currentNode->getChildNodes().value(i);
        QModelIndex index = this->getIndexForLink(link, column, nextNode);
        if(index.isValid())
        {
            return index;
        }
    }
    return QModelIndex();
}

QModelIndex TableModel::getIndexForTreeNode(TreeNode *treeNode, int column, TreeNode *currentNode) const
{
    if(currentNode == nullptr)
    {
       currentNode = this->rootNode;
    }
    if(treeNode == nullptr)
    {
        return QModelIndex();
    }
    if(currentNode == treeNode && currentNode != this->rootNode)
    {
        QModelIndex index = createIndex(currentNode->row(), column, currentNode);
        return index;
    }
    for(int i = 0; i < currentNode->getChildNodeCount(); i++)
    {
        TreeNode *nextNode = currentNode->getChildNodes().value(i);
        QModelIndex index = this->getIndexForTreeNode(treeNode, column, nextNode);
        if(index.isValid())
        {
            return index;
        }
    }
    return QModelIndex();
}

Link *TableModel::getUnprocessedLink(TreeNode *currentNode)
{
    if(currentNode == nullptr)
    {
       currentNode = this->rootNode;
    }
    if(currentNode != this->rootNode)
    {
        if(!currentNode->getLink()->getData(Link::DATA_IS_STARTED).toBool() && currentNode->getLink()->getData(Link::DATA_IS_CONTAINER).toBool() == false)
        {
            return currentNode->getLink();
        }
    }
    for(int i = 0; i < currentNode->getChildNodeCount(); i++)
    {
        TreeNode *nextNode = currentNode->getChildNodes().value(i);
        Link *link = this->getUnprocessedLink(nextNode);
        if(link && !link->getData(Link::DATA_IS_STARTED).toBool() && link->getData(Link::DATA_IS_CONTAINER).toBool() == false)
        {
            return link;
        }
    }
    return nullptr;
}

Link *TableModel::getParentLink(Link *link)
{
    TreeNode *linkNode = (TreeNode*) getIndexForLink(link, 0).internalPointer();
    return linkNode->getParentNode()->getLink();
}

void TableModel::updateLinkProgress(Link *link, QString progress)
{
    link->setData(Link::DATA_PROGRESS, progress);
    emit dataChanged(this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_PROGRESS)),
                     this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_PROGRESS)));
}

void TableModel::refreshName(Link *link, QString name)
{
    if(!name.isEmpty())
    {
        link->setData(Link::DATA_TITLE, QVariant(name));
    }
    emit dataChanged(this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_TITLE)),
                     this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_TITLE)));
}

QModelIndex TableModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    if(!parent.isValid())
    {
        return createIndex(row, column, this->rootNode->getChildNode(row));
    }

    TreeNode *parentNode = (TreeNode*) parent.internalPointer();
    return createIndex(row, column, parentNode->getChildNode(row));
}

QModelIndex TableModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }
    TreeNode *indexNode = (TreeNode*) index.internalPointer();
    if(indexNode->getParentNode() == this->rootNode)
    {
        return QModelIndex();
    }
    return createIndex(indexNode->getParentNode()->row(), 0, indexNode->getParentNode());
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column() > 0)
    {
        return 0;
    }
    if(!parent.isValid())
    {
        return this->rootNode->getChildNodeCount();
    }
    TreeNode *parentNode = (TreeNode*) parent.internalPointer();
    return parentNode->getChildNodeCount();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    return Link::DISPLAY_MAX_PROPERTIES;
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole && role != Qt::UserRole)
    {
        return QVariant();
    }
    TreeNode *indexNode = (TreeNode*) index.internalPointer();
    return indexNode->getLink()->getData(mapColumnToData(index.column()));
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions TableModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions TableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData *TableModel::mimeData(const QModelIndexList &indexes) const
{
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::ReadWrite);
    for(int i = 0; i < indexes.size(); i++)
    {
        QModelIndex index = indexes.at(i);
        quintptr address = (quintptr) index.internalPointer();
        stream << index.row() << index.column() << address;
    }
    QMimeData *mimeData = new QMimeData();
    mimeData->setData("application/x-qabstractitemmodeldatalist", encodedData);
    return mimeData;
}

bool TableModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    TreeNode *lastTreeNode = nullptr;

    while(!stream.atEnd())
    {
        int sourceRow, sourceCol;
        quintptr encodedTreeNode;
        stream >> sourceRow >> sourceCol >> encodedTreeNode;
        TreeNode *treeNode = (TreeNode*) encodedTreeNode;
        if(sourceCol > 0)
        {
            continue;
        }
        qDebug() << "LinkTitle: " << treeNode->getLink()->getData(Link::DATA_TITLE) << "SouceRow: " << treeNode->row() << "targetRow: " << row;
        int finalRow = row;
        if(lastTreeNode)
        {
            finalRow = lastTreeNode->row() + 1;
        }
        moveRows(getIndexForTreeNode(treeNode->getParentNode(), 0), treeNode->row(), 1, parent, finalRow);
        lastTreeNode = treeNode;
    }
    return true;
}



