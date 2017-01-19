#ifndef TREEVIEW_H
#define TREEVIEW_H
#include <QTreeView>
#include <QDropEvent>
#include <QDebug>

class TreeView : public QTreeView
{
    Q_OBJECT

public:
    TreeView(QWidget *parent = Q_NULLPTR);

    void dropEvent(QDropEvent *event);


};

#endif // TREEVIEW_H
