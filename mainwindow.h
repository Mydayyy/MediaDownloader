#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QTreeView>
#include <QTimer>
#include <QLineEdit>
#include <QEventLoop>
#include <QAction>
#include <QAction>
#include <QDir>

#include "tablemodel.h"
#include "youtubedownloader.h"
#include "windowsettings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    TableModel *tableModel;
    YoutubeDownloader *youtube;

    void setupTableModel();

public slots:
    void tableViewCustomContextMenu(QPoint pos);
    void actionPlayTriggered(bool checked);

private slots:
    void on_buttonAddTrack_clicked();
    void on_actionSettings_triggered();

};

#endif // MAINWINDOW_H
