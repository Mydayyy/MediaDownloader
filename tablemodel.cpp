#include "tablemodel.h"

TableModel::TableModel()
{

}

TableModel::~TableModel()
{
    QMap<Link*, QList<Link*>>::iterator i;
    for(i = this->subItems.begin(); i != this->subItems.end(); ++i)
    {
        while(i.value().size() > 0)
        {
            delete i.value().takeAt(0);
        }
    }
    this->subItems.clear();

    for(int i = 0; i < this->mLinks.size(); i++)
    {
        delete this->mLinks[i];
    }
    this->mLinks.clear();
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
