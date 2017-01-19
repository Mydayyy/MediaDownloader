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

    Link *addLink(QString link, Link *parent = nullptr);
    Link *addLink(Link *link, Link *parent = nullptr);
    void removeLink(Link *link) = delete; // CONSIDER / REMOVE
    void deleteLink(Link *link);
    void deleteIndexRow(QModelIndex index);
    Link *getUnprocessedLink(const QModelIndex &parent = QModelIndex());

public slots:
    void updateLinkProgress(Link *link, QString progress);
    void updateName(Link *link, QString newName);
    void refreshName(Link *link);

private:
    QList<Link*> mLinks;
    QMap<Link*, QList<Link*>> subItems;



    // Needed for model
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex getIndexForLink(Link *link, int column = 0) const;
    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex indexParent(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);
    bool myRemoveRows(int row, int count, const QModelIndex &parent);

    Qt::DropActions supportedDragActions() const;
    Qt::DropActions supportedDropActions() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    //-----------------------------


};

#endif // TABLEMODEL_H
