#ifndef TREEVIEWMEDIAOBJECTSETTINGS_H
#define TREEVIEWMEDIAOBJECTSETTINGS_H

#include <QWidget>

namespace Ui {
class TreeViewMediaObjectSettings;
}

class TreeViewMediaObjectSettings : public QWidget
{
    Q_OBJECT

public:
    explicit TreeViewMediaObjectSettings(QWidget *parent = 0);
    ~TreeViewMediaObjectSettings();

private:
    Ui::TreeViewMediaObjectSettings *ui;
};

#endif // TREEVIEWMEDIAOBJECTSETTINGS_H
