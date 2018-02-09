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
#include <QStringList>
#include <QClipboard>
#include <QPushButton>
#include <QBoxLayout>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>


#include "mediaobject.h"
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
    QClipboard *clipboard;
    QString lastClipboard;

    void setupTableModel();

    void setSettingForCurrentObjects(QString key, QVariant val);
    void updateSettingPanel();

public slots:
    void tableViewCustomContextMenu(QPoint pos);
    void actionPlayTriggered(bool checked);

private slots:
    void on_buttonAddTrack_clicked();
    void on_actionSettings_triggered();

    void on_actionClipboard_Watchdog_toggled(bool checked);
    void onClipboardChanged();
    void on_actionAnalyse_clipboard_for_links_triggered();
    void on_actionAbout_triggered();
    void on_buttonBrowseFiles_clicked();

    void onSelectionChanged(const QItemSelection& deselected, const QItemSelection& selected);
    void on_inputSavePath_textEdited(const QString &text);
    void on_extractOnlyAudio_toggled(bool checked);
    void on_audioFormat_currentTextChanged(const QString &arg1);
};

#endif // MAINWINDOW_H
