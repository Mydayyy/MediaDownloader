#include "tablemodel.h"

TableModel::TableModel()
{

}

TableModel::~TableModel()
{

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

Link* TableModel::addLink(QString link, Link* parent)
{
    Link *mediaLink = new Link(link, link, "0%");
    return this->addLink(mediaLink, parent);
}

Link *TableModel::addLink(Link *link, Link *parent)
{
    if(parent)
    {
        if(this->getIndexForLink(parent).parent().isValid())
        {
            parent = static_cast<Link*>(this->getIndexForLink(parent).parent().internalPointer());
        }
        if(!parent->getData(Link::DATA_IS_CONTAINER).toBool())
        {
            int nonContainerIndex = this->mLinks.indexOf(parent);

            beginRemoveRows(QModelIndex(), nonContainerIndex, nonContainerIndex);
            this->mLinks.removeAt(nonContainerIndex);
            this->subItems.remove(parent);
            endRemoveRows();

            Link *container = new Link("Lorem Ipsum Dolor Sit Amet", "", "");
            container->setData(Link::DATA_IS_CONTAINER, QVariant(true));

            beginInsertRows(QModelIndex(), nonContainerIndex, nonContainerIndex);
            this->mLinks.insert(nonContainerIndex, container);
            this->subItems[container].append(parent);
            endInsertRows();

            parent = container;
        }
        int index = this->mLinks.indexOf(parent);
        this->subItems[parent];
        beginInsertRows(this->index(index, 0, createIndex(index, 0, this->mLinks[index])), this->subItems[parent].size(), this->subItems[parent].size());
        this->subItems[parent].append(link);
        endInsertRows();
    }
    else
    {
        beginInsertRows(QModelIndex(), mLinks.size(), mLinks.size());
        this->mLinks.append(link);
        endInsertRows();
    }
    return link;
}


void TableModel::deleteLink(Link *link)
{
    QModelIndex index = this->getIndexForLink(link);
    this->myRemoveRows(index.row(), 1, index.parent());
}

void TableModel::deleteIndexRow(QModelIndex index)
{
    this->deleteLink(static_cast<Link*>(index.internalPointer()));
}

Link* TableModel::getUnprocessedLink(const QModelIndex &parent)
{
    if(parent.isValid())
    {
        Link *link = static_cast<Link*>(parent.internalPointer());
        for(int i = 0; i < this->subItems[link].size(); i++)
        {
            if(!this->subItems[link][i]->getData(Link::DATA_IS_STARTED).toBool())
            {
                return this->subItems[link][i];
            }
        }
    }
    else
    {
        for(int i = 0;i < this->mLinks.size(); i++)
        {
            if(this->mLinks[i]->getData(Link::DATA_IS_CONTAINER).toBool())
            {
                Link *link = this->getUnprocessedLink(this->getIndexForLink(this->mLinks[i]));
                if(link)
                {
                    return link;
                }
            }
            else if(!this->mLinks[i]->getData(Link::DATA_IS_STARTED).toBool())
            {
                 return this->mLinks[i];
            }
        }
    }
    return nullptr;
}

void TableModel::updateLinkProgress(Link *link, QString progress)
{
    link->setData(Link::DATA_PROGRESS, progress);
    emit dataChanged(this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_PROGRESS)),
                     this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_PROGRESS)));
}

void TableModel::updateName(Link *link, QString newName)
{
    link->setData(Link::DATA_TITLE, newName);
    emit dataChanged(this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_TITLE)),
                     this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_TITLE)));
}

void TableModel::refreshName(Link *link)
{
    emit dataChanged(this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_TITLE)),
                     this->getIndexForLink(link, this->mapDataToColumn(Link::DATA_TITLE)));
}

// FUNCTIONS NEEDED FOR QABSTRACTTABLEMODEL
QModelIndex TableModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    Link *link;
    if(!parent.isValid())
    {
        return createIndex(row, column, this->mLinks[row]);
    }
    else
    {
        link = static_cast<Link*>(parent.internalPointer());
        if(!this->subItems.contains(link) || this->subItems[link].size()- 1 < row)
        {
            return QModelIndex();
        }
        return createIndex(row, column, this->subItems[link][row]);
    }
}

QModelIndex TableModel::getIndexForLink(Link *link, int column) const
{
    if(this->mLinks.contains(link))
    {
        int indexOf = this->mLinks.indexOf(link);
        QModelIndex index =  this->index(indexOf, column, QModelIndex());
        return index;
    }

    QMap<Link*, QList<Link*>>::const_iterator i;
    for(i = this->subItems.begin(); i != this->subItems.end(); ++i)
    {
        if(i.value().contains(link))
        {
            int indexOf = i.value().indexOf(link);
            QModelIndex parent = this->index(this->mLinks.indexOf(i.key()), column, QModelIndex());
            QModelIndex index = this->index(indexOf, column, parent);
            return index;
        }
    }
    return QModelIndex();
}

QModelIndex TableModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }

    Link *currentItem = static_cast<Link*>(index.internalPointer());
    if(this->mLinks.contains(currentItem)) {
        return QModelIndex();
    }
    QMap<Link*, QList<Link*>>::const_iterator i;
    for(i = this->subItems.begin(); i != this->subItems.end(); ++i)
    {
        int indexOfChild = i.value().indexOf(currentItem);
        if(indexOfChild == -1)
        {
            continue;
        }
        int indexOfParent = this->mLinks.indexOf(i.key());
        return createIndex(indexOfParent, 0, this->mLinks[indexOfParent]);
    }
    return QModelIndex();
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    Link *link;
    if(parent.column() > 0)
    {
        return 0;
    }
    if(parent.isValid())
    {
        link = static_cast<Link*>(parent.internalPointer());
        return this->subItems[link].size();
    }
    else
    {
        return this->mLinks.size();
    }
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    return Link::DISPLAY_MAX_PROPERTIES;
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if(index.parent().isValid())
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::UserRole || role == Qt::DisplayRole)
    {
        if(index.parent().isValid())
        {
            Link *parentLink = static_cast<Link*>(index.parent().internalPointer());
            return this->subItems[parentLink].at(index.row())->getData(mapColumnToData(index.column()));
        }
        return mLinks.at(index.row())->getData(mapColumnToData(index.column()));
    }
    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == -1)
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch(mapColumnToData(section))
            {
            case Link::DATA_TITLE: return "Title";
            case Link::DATA_PROGRESS: return "Progress";
            default: return "";
            }
        }
        else
        {
            return QString::number(section+1);
        }
    }
    return QVariant();
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role != Qt::DisplayRole)
    {
        return QAbstractItemModel::setData(index, value, role);;
    }

    if(index.parent().isValid())
    {
        Link *parentLink = static_cast<Link*>(index.parent().internalPointer());
        int indexOf = index.row();
        this->subItems[parentLink][indexOf]->setData(mapColumnToData(index.column()), value);
        emit dataChanged(createIndex(index.row(), index.column()), createIndex(index.row(), index.column()));
    }
    else
    {
        this->mLinks[index.row()]->setData(mapColumnToData(index.column()), value);
        emit dataChanged(createIndex(index.row(), index.column()), createIndex(index.row(), index.column()));
    }
    return true;
}

bool TableModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    QMap<int, QVariant>::const_iterator i;
    for(i = roles.begin(); i != roles.end(); i++)
    {
        this->setData(index, i.value(), i.key());
    }
    return true;
}

bool TableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    QList<Link*> itemsToMove;
    if(destinationParent.isValid())
    {
        if(destinationChild == -1)
        {
            destinationChild = this->subItems[static_cast<Link*>(destinationParent.internalPointer())].size();
        }
    }
    else
    {
        if(destinationChild == -1)
        {
            destinationChild = this->mLinks.size();
        }
    }


    bool con = false;
    if(destinationParent == sourceParent)
    {
        if(sourceParent == destinationParent && destinationChild >= sourceRow && destinationChild <= sourceRow+count) { return false; }
        con = beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild);
    }
    else
    {
        con = beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild);
    }
    if(con == false)
    {
        //return false;
    }
    int sizeModifier = count;
    if(sourceParent.isValid())
    {
        Link *sourceLink = static_cast<Link*>(sourceParent.internalPointer());
        while(count)
        {
            itemsToMove.append( this->subItems[sourceLink].takeAt(sourceRow) );
            count--;
        }
    }
    else
    {
        while(count)
        {
            itemsToMove.append( this->mLinks.takeAt(sourceRow) );
            count--;
        }
    }
    if(itemsToMove.size() > 0)
    {
        if(destinationParent.isValid())
        {
            Link *destinationLink = static_cast<Link*>(destinationParent.internalPointer());
            if(sourceParent == destinationParent && sourceRow < destinationChild)
            {
                destinationChild -= sizeModifier;
            }
            while(itemsToMove.size() > 0)
            {

                this->subItems[destinationLink].insert(destinationChild, itemsToMove.takeLast());
            }

        }
        else
        {
            if(sourceParent == destinationParent && sourceRow < destinationChild)
            {
                destinationChild -= sizeModifier;
            }
            while(itemsToMove.size() > 0)
            {
                this->mLinks.insert(destinationChild, itemsToMove.takeLast());
            }
        }
    }
    endMoveRows();
    return true;
}

// TODO: REWRITE removeAt to takeAt
bool TableModel::myRemoveRows(int row, int count, const QModelIndex &parent)
{
    this->beginRemoveRows(parent, row, row+count-1);
    if(parent.isValid())
    {
        Link *parentLink = static_cast<Link*>(parent.internalPointer());
        while(count > 0)
        {
            Link *link = this->subItems[parentLink][row];
            this->subItems[parentLink].removeAt(row);
            count--;
            delete link;
        }
    }
    else
    {
        while(count > 0)
        {
            Link *link = this->mLinks[row];
            this->mLinks.removeAt(row);
            count--;

            if(this->subItems.contains(link))
            {
                while(this->subItems[link].size() > 0)
                {
                    Link *subLink = this->subItems[link][0];
                    this->subItems[link].removeAt(0);
                    delete subLink;
                }
                this->subItems.remove(link);
            }
            delete link;
        }
    }
    this->endRemoveRows();
    return true;
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
    QList<Link*> alreadyMoved;
    QModelIndex realParent = parent;
    int counter = 0;
    Link *lastLink = nullptr;
    while (!stream.atEnd())
    {
        int sourceRow, sourceCol;
        quintptr encodedLink;
        stream >> sourceRow >> sourceCol >> encodedLink;
        if(sourceCol > 0)
        {
            continue;
        }
        Link *link = (Link*) encodedLink;
        bool isContainer = link->getData(Link::DATA_IS_CONTAINER).toBool();
        if(realParent.isValid())
        {
            Link *parentLink = static_cast<Link*>(realParent.internalPointer());
            if(!parentLink->getData(Link::DATA_IS_CONTAINER).toBool())
            {
                int nonContainerIndex = this->mLinks.indexOf(parentLink);

                beginRemoveRows(QModelIndex(), nonContainerIndex, nonContainerIndex);
                this->mLinks.removeAt(nonContainerIndex);
                this->subItems.remove(parentLink);
                endRemoveRows();

                Link *container = new Link("Lorem Ipsum Dolor Sit Amet", "", "");
                container->setData(Link::DATA_IS_CONTAINER, QVariant(true));

                beginInsertRows(QModelIndex(), nonContainerIndex, nonContainerIndex);
                this->mLinks.insert(nonContainerIndex, container);
                this->subItems[container].append(parentLink);
                endInsertRows();

                realParent = this->index(this->mLinks.indexOf(container), 0, QModelIndex());
            }
            if(isContainer)
            {
                while(this->subItems[link].size() > 0)
                {
                    Link *containerItem = this->subItems[link][0];
                    if(lastLink && row != -1)
                    {
                        row = this->subItems[parentLink].indexOf(lastLink)+1;
                    }
                    QModelIndex containerItemIndex = this->getIndexForLink(containerItem);
                    lastLink = containerItem;
                    this->moveRows(containerItemIndex.parent(), 0, 1, realParent, row);
                }
                continue;
            }

            QModelIndex linkIndex = this->getIndexForLink(link);
            if(linkIndex.parent().isValid())
            {
                Link *linkParent = static_cast<Link*>(linkIndex.parent().internalPointer());
                sourceRow = this->subItems[linkParent].indexOf(link);
            }
            else
            {
                sourceRow = this->mLinks.indexOf(link);
            }
            if(lastLink && row != -1)
            {
                row = this->subItems[parentLink].indexOf(lastLink)+1;
            }
            lastLink = link;
            this->moveRows(linkIndex.parent(), sourceRow, 1, realParent, row);
        }
        else
        {
            QModelIndex linkIndex = this->getIndexForLink(link);
            if(linkIndex.parent().isValid())
            {
                Link *linkParent = static_cast<Link*>(linkIndex.parent().internalPointer());
                sourceRow = this->subItems[linkParent].indexOf(link);
            }
            else
            {
                sourceRow = this->mLinks.indexOf(link);
            }
            if(lastLink && row != -1)
            {
                row = this->mLinks.indexOf(lastLink)+1;
            }
            lastLink = link;
            this->moveRows(linkIndex.parent(), sourceRow, 1, parent, row);
            continue;
        }
    }
    QMutableListIterator<Link*> i(this->mLinks);
    while (i.hasNext()) {
        Link *link = i.next();
        if(link->getData(Link::DATA_IS_CONTAINER).toBool() && this->subItems[link].size() <= 0)
        {
            beginRemoveRows(QModelIndex(), this->mLinks.indexOf(link), this->mLinks.indexOf(link));
            i.remove();
            endRemoveRows();
        }
    }
    return true;
}

//---------------------------------------------------------
