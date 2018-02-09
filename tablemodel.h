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
#include <QDropEvent>

#include "mediaobject.h"
#include "treenode.h"



class TableModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    TableModel();
    ~TableModel();

    static int mapDataToColumn(MediaObject::Data data);
    static MediaObject::Data mapColumnToData(int column);

    //Helper functions to easily manipulate the table
    MediaObject *addLink(QString link, MediaObject *parent = nullptr);
    MediaObject *addLink(MediaObject *link, MediaObject *parent = nullptr);

    void deleteIndex(QModelIndex index);
    bool myRemoveRows(int row, int count, const QModelIndex &parent);

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow);

    void convertToContainer(const QModelIndex &index);

    // Helper functions to easily retrieve data
    QModelIndex getIndexForLink(MediaObject *link, int column, TreeNode *currentNode = nullptr) const;
    QModelIndex getIndexForTreeNode(TreeNode *treeNode, int column, TreeNode *currentNode = nullptr) const;
    MediaObject getLinkForIndex(const QModelIndex &index);
    QList<TreeNode *> convertIndexListToTreeNodeList(const QModelIndexList list);

    // STUBS
    MediaObject *getUnprocessedLink(TreeNode *currentNode = nullptr);
    MediaObject *getParentLink(MediaObject *link);
    void updateLinkProgress(MediaObject *link);
    void updateProgressCell(MediaObject *link);
    void refreshName(MediaObject *link, QString name = "");
    void redrawMediaObject(MediaObject *mo);


    // The following functions are required to be implemented and belong to the table model
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;

    // The following functions are required to support drag and drop inside the view
    Qt::ItemFlags flags(const QModelIndex &index) const;
    Qt::DropActions supportedDragActions() const;
    Qt::DropActions supportedDropActions() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

private:
    TreeNode *rootNode;
};

#endif // TABLEMODEL_H
