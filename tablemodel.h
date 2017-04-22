#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QMap>
#include <QDebug>
#include <QMapIterator>
#include <QMimeData>
#include <QMenu>
#include <QMutableListIterator>

#include "link.h"
#include "QDropEvent"


class TableModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    TableModel();
    ~TableModel();

    static int mapDataToColumn(Link::Data data);
    static Link::Data mapColumnToData(int column);

    // The following functions are required to be implemented
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
};

#endif // TABLEMODEL_H
