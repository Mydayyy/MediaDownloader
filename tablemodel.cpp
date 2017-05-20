#include "tablemodel.h"

TableModel::TableModel()
{
    rootNode = new TreeNode();
}

TableModel::~TableModel()
{
    delete rootNode;
}

Link::Data TableModel::mapColumnToData(int column)
{
    switch (column) {
    case 0: return Link::DATA_TITLE;
    case 1: return Link::DATA_PROGRESS;
    }
    return Link::DATA_INVALID;
}

int TableModel::mapDataToColumn(Link::Data data)
{
    switch (data) {
    case Link::DATA_TITLE: return 0;
    case Link::DATA_PROGRESS: return 1;
    default: return 0;
    }
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
    TreeNode *parentNode = (TreeNode*) parentIndex.internalPointer();
    if(!parentNode->getLink()->getData(Link::Data::DATA_IS_CONTAINER).toBool())
    {
        Link *newLink = new Link(*(parentNode->getLink()));
        parentNode->getLink()->setData(Link::DATA_IS_CONTAINER, QVariant(true));
        beginInsertRows(parentIndex, parentNode->getChildNodeCount(), parentNode->getChildNodeCount());
        parentNode->appendChildNode(new TreeNode(newLink, parentNode));
        endInsertRows();
    }
    beginInsertRows(parentIndex, parentNode->getChildNodeCount(), parentNode->getChildNodeCount());
    parentNode->appendChildNode(new TreeNode(link, parentNode));
    endInsertRows();
    return link;
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

void TableModel::deleteIndex(QModelIndex index)
{
    if(!index.isValid())
    {
        return;
    }
    this->myRemoveRows(index.row(), 1, index.parent());
}

bool TableModel::myRemoveRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row+count-1);
    TreeNode *parentNode;// = (TreeNode*) parent.internalPointer();
    if(parent.isValid())
    {
        parentNode = (TreeNode*) parent.internalPointer();
    }
    else
    {
        parentNode = rootNode;
    }
    while(count > 0)
    {
        parentNode->deleteChildNode(row);
        count--;
    }

    this->endRemoveRows();
    return true;
}

bool TableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow)
{
    qDebug() << sourceRow;
    if(count <= 0)
    {
        return false;
    }
    TreeNode *sourceParentNode = nullptr;
    if(!sourceParent.isValid())
    {
        sourceParentNode = this->rootNode;
    }
    else
    {
        sourceParentNode = (TreeNode*) sourceParent.internalPointer();
    }

    TreeNode *destinationParentNode = nullptr;
    if(!destinationParent.isValid())
    {
        destinationParentNode = this->rootNode;
    }
    else
    {
        destinationParentNode = (TreeNode*) destinationParent.internalPointer();
    }

    beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationRow);
    for(int i = count; i > 0; i--)
    {
        TreeNode *item = sourceParentNode->getChildNode(sourceRow);
        sourceParentNode->removeChildNode(sourceRow);
        destinationParentNode->insertChildNode(item, destinationRow);
    }
    endMoveRows();
    return true;
}

Link *TableModel::getUnprocessedLink()
{
    return new Link("", "", "");
}

Link *TableModel::getParentLink(Link *link)
{
    return new Link("", "", "");
}

void TableModel::updateLinkProgress(Link *link, QString progress)
{

}

void TableModel::refreshName(Link *link)
{

}

QModelIndex TableModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    TreeNode *parentNode;

    if(!parent.isValid())
    {
        parentNode = rootNode;
    }
    else
    {
        parentNode = (TreeNode*) parent.internalPointer();
    }

    TreeNode *childItem = parentNode->getChildNode(row);
    if(childItem)
    {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex TableModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }

    TreeNode *childNode = (TreeNode*) index.internalPointer();
    TreeNode *parentNode = (TreeNode*) childNode->getParentNode();

    if(parentNode == rootNode)
    {
        return QModelIndex();
    }

    return createIndex(parentNode->row(), 0, parentNode);
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    TreeNode *parentNode;
    if(parent.column() > 0)
    {
        return 0;
    }
    if(!parent.isValid())
    {
        parentNode = rootNode;
    }
    else
    {
        parentNode = (TreeNode*) parent.internalPointer();
    }

    return parentNode->getChildNodeCount();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return ((TreeNode*) parent.internalPointer())->columnCount();
    }
    return rootNode->columnCount();
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
    {
        return QVariant();
    }

    if(role != Qt::DisplayRole)
    {
        return QVariant();
    }

    TreeNode *item = (TreeNode*) index.internalPointer();
    return item->data(index.column());
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
    QList<TreeNode*> alreadyMoved;
    QModelIndex dropToParent = parent;
    TreeNode *lastTreeNode = nullptr;

    while(!stream.atEnd())
    {
        int sourceRow, sourceCol;
        quintptr encodedTreeNode;
        stream >> sourceRow >> sourceCol >> encodedTreeNode;
        if(sourceCol > 0)
        {
            continue;
        }
        TreeNode *treeNode = (TreeNode*) encodedTreeNode;
        TreeNode *parentNode = treeNode->getParentNode();

        moveRows(getIndexForTreeNode(parentNode, 0), sourceRow, 1, dropToParent, row);
    }
    return true;
}


