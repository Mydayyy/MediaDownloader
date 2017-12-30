#include "treeviewmediaobjectsettings.h"
#include "ui_treeviewmediaobjectsettings.h"

TreeViewMediaObjectSettings::TreeViewMediaObjectSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TreeViewMediaObjectSettings)
{
    ui->setupUi(this);
}

TreeViewMediaObjectSettings::~TreeViewMediaObjectSettings()
{
    delete ui;
}
