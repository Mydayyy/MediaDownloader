#ifndef MEDIATREEVIEW_H
#define MEDIATREEVIEW_H

#include <QObject>
#include <QWidget>
#include <QTreeView>
#include <QScrollArea>
#include <QScrollBar>
#include <QHeaderView>
#include <QDebug>
#include <QTimer>

class MediaTreeView : public QTreeView
{
public:
    explicit MediaTreeView(QWidget *parent = Q_NULLPTR);

    // QAbstractScrollArea interface
protected:


    // QAbstractScrollArea interface
protected:
    QSize viewportSizeHint() const override;

    // QAbstractItemView interface
protected slots:
    void updateGeometries();
};

#endif // MEDIATREEVIEW_H
