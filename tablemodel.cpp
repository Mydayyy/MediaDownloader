#include "tablemodel.h"

TableModel::TableModel()
{
    rootNode = new TreeNode(new MediaObject("", "" ,""));
    rootNode->getLink()->setData(MediaObject::DATA_IS_CONTAINER, QVariant(true));
}

TableModel::~TableModel()
{
    delete rootNode;
}

int TableModel::mapDataToColumn(MediaObject::Data data)
{
    switch (data) {
    case MediaObject::DATA_TITLE: return 0;
    case MediaObject::DATA_PROGRESS: return 1;
    default: return 0;
    }
}

MediaObject::Data TableModel::mapColumnToData(int column)
{
    switch (column) {
    case 0: return MediaObject::DATA_TITLE;
    case 1: return MediaObject::DATA_PROGRESS;
    }
    return MediaObject::DATA_INVALID;
}

MediaObject *TableModel::addLink(QString link, MediaObject *parent)
{
    MediaObject *mediaLink = new MediaObject(link, link, "0%");
    return this->addLink(mediaLink, parent);
}

MediaObject *TableModel::addLink(MediaObject *link, MediaObject *parent)
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
    return true;
}

void TableModel::convertToContainer(const QModelIndex &index)
{
    TreeNode *indexNode = (TreeNode*) index.internalPointer();
    if(!index.isValid())
    {
        indexNode = this->rootNode;
    }

    if(!indexNode->getLink()->getData(MediaObject::Data::DATA_IS_CONTAINER).toBool())
    {
        MediaObject *newLink = new MediaObject(*(indexNode->getLink()));
        indexNode->getLink()->setData(MediaObject::DATA_IS_CONTAINER, QVariant(true));
        updateLinkProgress(indexNode->getLink());
        refreshName(indexNode->getLink(), "Container");
        indexNode->getLink()->setData(MediaObject::DATA_IS_CONTAINER, QVariant(true));
        beginInsertRows(index, indexNode->getChildNodeCount(), indexNode->getChildNodeCount());
        indexNode->appendChildNode(new TreeNode(newLink, indexNode));
        endInsertRows();
    }
}

QModelIndex TableModel::getIndexForLink(MediaObject *link, int column) const
{
    if(link == nullptr)
    {
        return QModelIndex();
    }
    return this->getIndexForTreeNode(link->getTreeNode(), column);
}

QModelIndex TableModel::getIndexForTreeNode(TreeNode *treeNode, int column) const
{
    if(treeNode == nullptr || treeNode == this->rootNode)
    {
        return QModelIndex();
    }
    QModelIndex index = createIndex(treeNode->row(), column, treeNode);
    return index;
}

QList<TreeNode *> TableModel::convertIndexListToTreeNodeList(const QModelIndexList list)
{
    QList<TreeNode*> treeNodeList;

    for(auto it = list.begin(); it != list.end(); it++) {
        QModelIndex item = *it;
        TreeNode *linkNode = (TreeNode*) item.internalPointer();
        if(treeNodeList.contains(linkNode)) {
            continue;
        }
        treeNodeList.append(linkNode);
    }
    return treeNodeList;
}

MediaObject *TableModel::getUnprocessedLink(TreeNode *currentNode)
{
    if(currentNode == nullptr)
    {
       currentNode = this->rootNode;
    }
    if(currentNode != this->rootNode)
    {
        if(!currentNode->getLink()->getData(MediaObject::DATA_IS_STARTED).toBool() && currentNode->getLink()->getData(MediaObject::DATA_IS_CONTAINER).toBool() == false)
        {
            return currentNode->getLink();
        }
    }
    for(int i = 0; i < currentNode->getChildNodeCount(); i++)
    {
        TreeNode *nextNode = currentNode->getChildNodes().value(i);
        MediaObject *link = this->getUnprocessedLink(nextNode);
        if(link && !link->getData(MediaObject::DATA_IS_STARTED).toBool() && link->getData(MediaObject::DATA_IS_CONTAINER).toBool() == false)
        {
            return link;
        }
    }
    return nullptr;
}

MediaObject *TableModel::getParentLink(MediaObject *link)
{
    TreeNode *linkNode = (TreeNode*) getIndexForLink(link, 0).internalPointer();
    return linkNode->getParentNode()->getLink();
}

void TableModel::updateLinkProgress(MediaObject *link)
{
    if(link->getData(MediaObject::DATA_IS_CONTAINER).toBool()) {
        link->setData(MediaObject::DATA_PROGRESS, "");
        this->updateProgressCell(link);
        return;
    }

    if(link->getData(MediaObject::DATA_IS_FAILED).toBool()) {
        link->setData(MediaObject::DATA_PROGRESS, "FAILED");
        this->updateProgressCell(link);
        return;
    }

    if(link->getData(MediaObject::DATA_IS_SKIPPED).toBool()) {
        link->setData(MediaObject::DATA_PROGRESS, "SKIPPED");
        this->updateProgressCell(link);
        return;
    }

    if(link->getData(MediaObject::DATA_IS_STARTED).toBool() && !link->getData(MediaObject::DATA_IS_FINISHED).toBool()) {
        QString speed = link->getData(MediaObject::DATA_SPEED).toString();
        QString maxSize = link->getData(MediaObject::DATA_MAX_SIZE).toString();
        QString time = link->getData(MediaObject::DATA_TIME).toString();
        QString progress = link->getData(MediaObject::DATA_DOWNLOADED_SIZE).toString();
        QString str = ""+progress+" of "+maxSize+" at "+speed+" ( "+time+" )";
        link->setData(MediaObject::DATA_PROGRESS, str);
        this->updateProgressCell(link);
        return;
    }

    if(link->getData(MediaObject::DATA_IS_STARTED).toBool() == false && link->getData(MediaObject::DATA_IS_FINISHED).toBool() == false) {
        link->setData(MediaObject::DATA_PROGRESS, "0%");
        this->updateProgressCell(link);
        return;
    }

    if(link->getData(MediaObject::DATA_IS_FINISHED).toBool()) {
        QString time = link->getData(MediaObject::DATA_TIME).toString();
        QString maxSize = link->getData(MediaObject::DATA_MAX_SIZE).toString();
        QString str = "100% of " + maxSize;
        if(!time.isEmpty()) {
            str += "  in " + time;
        }
        link->setData(MediaObject::DATA_PROGRESS, str);
        this->updateProgressCell(link);
        return;
    }
}

void TableModel::updateProgressCell(MediaObject *link)
{
    emit dataChanged(this->getIndexForLink(link, this->mapDataToColumn(MediaObject::DATA_PROGRESS)),
                     this->getIndexForLink(link, this->mapDataToColumn(MediaObject::DATA_PROGRESS)));
}

void TableModel::refreshName(MediaObject *link, QString name)
{
    if(!name.isEmpty())
    {
        link->setData(MediaObject::DATA_TITLE, QVariant(name));
    }
    emit dataChanged(this->getIndexForLink(link, this->mapDataToColumn(MediaObject::DATA_TITLE)),
                     this->getIndexForLink(link, this->mapDataToColumn(MediaObject::DATA_TITLE)));
}

void TableModel::redrawMediaObject(MediaObject *mo)
{
    emit dataChanged(this->getIndexForLink(mo, 0),
                     this->getIndexForLink(mo, MediaObject::DISPLAY_MAX_PROPERTIES - 1));
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
    return MediaObject::DISPLAY_MAX_PROPERTIES;
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == -1)
        return QVariant();
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)      // we only have a horizontal header
            switch (mapColumnToData(section)) {
            case MediaObject::DATA_TITLE: return "Title";
            case MediaObject::DATA_PROGRESS: return "Progress";
            default: return "";
            }
        else    // vertical will be filled with index number (increasing)
            return QString::number(section+1);
    }
    return QVariant();
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
        //qDebug() << "LinkTitle: " << treeNode->getLink()->getData(Link::DATA_TITLE) << "SouceRow: " << treeNode->row() << "targetRow: " << row;
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



