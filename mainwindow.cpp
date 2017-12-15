#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // We want all actions be relative to the path where the application was started
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    // Starts downloading all currently added videos and all videos added while still in download mode
    QAction *actionPlay = ui->toolBar->addAction(QIcon(":/toolbar/icons/play.png"), tr("Play"));
    connect(actionPlay, SIGNAL(triggered(bool)), this, SLOT(actionPlayTriggered(bool)));

    // Unused for now. TODO: Implement
    ui->toolBar->addAction(QIcon(":/toolbar/icons/pause.png"), tr("Pause"));
    ui->toolBar->addAction(QIcon(":/toolbar/icons/stop.png"), tr("Stop"));

    this->setupTableModel();
    this->youtube = new YoutubeDownloader(this->tableModel);
    this->clipboard = QApplication::clipboard();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete this->youtube;
    delete this->tableModel;

}

void MainWindow::setupTableModel()
{
    this->tableModel = new TableModel();
    this->ui->treeTrackView->header()->setSectionResizeMode(QHeaderView::Stretch); // This will set all rows to equal length
    this->ui->treeTrackView->setModel(this->tableModel);

    // Enable drag and drop
    ui->treeTrackView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeTrackView->setDragEnabled(true);
    ui->treeTrackView->viewport()->setAcceptDrops(true);
    ui->treeTrackView->setDropIndicatorShown(true);
    ui->treeTrackView->setDragDropMode(QAbstractItemView::DragDrop);
    ui->treeTrackView->setDefaultDropAction(Qt::MoveAction);
    ui->treeTrackView->setContextMenuPolicy(Qt::CustomContextMenu);


//    Link *parentLink1 = this->tableModel->addLink("Link1");
//        this->tableModel->addLink("Link1.1", parentLink1);
//    Link *parentLink11 = this->tableModel->addLink("Link1.2", parentLink1);
//            this->tableModel->addLink("Link1.2.1", parentLink11);
//            this->tableModel->addLink("Link1.2.2", parentLink11);
//            this->tableModel->addLink("Link1.2.3", parentLink11);
//        this->tableModel->addLink("Link1.3", parentLink1);
//    this->tableModel->addLink("Link2");
//    this->tableModel->addLink("Link3");
//    this->tableModel->addLink("Link4");

    connect(ui->treeTrackView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableViewCustomContextMenu(QPoint))); // Custom Context Menu
}

void MainWindow::tableViewCustomContextMenu(QPoint pos)
{
    // TODO: We ( usually ) want context actions to apply to all selected items, so we need to iterate over them here and apply the action to each of them
    if(!ui->treeTrackView->indexAt(pos).isValid())
    {
        return;
    }
    QModelIndex selectedIndex = ui->treeTrackView->indexAt(pos);
    MediaObject *link = static_cast<MediaObject*>(selectedIndex.internalPointer());

    QMenu menu(this);

    QAction *deleteRow = new QAction("Delete", &menu); // Deletes an entry
    menu.addAction(deleteRow);

    QAction *retryLink = new QAction("Retry", &menu);
    if(link->getData(MediaObject::DATA_IS_FAILED).toBool())
    {
        menu.addAction(retryLink);
    }

    QAction *selectedAction = menu.exec(ui->treeTrackView->viewport()->mapToGlobal(pos));

    if(selectedAction == deleteRow)
    {
        this->tableModel->deleteIndex(selectedIndex);
        return;
    }
    if(selectedAction == retryLink)
    {
        link->setData(MediaObject::DATA_IS_FAILED, QVariant(false));
        link->setData(MediaObject::DATA_IS_STARTED, QVariant(false));
        return;
    }
}



void MainWindow::on_buttonAddTrack_clicked()
{
    this->youtube->addLink(ui->inputUrl->text());
}

void MainWindow::on_actionSettings_triggered()
{
    WindowSettings *ws = new WindowSettings(this);
    int startX = 0, startY = 0;
    startX = int (this->geometry().x() + this->geometry().width() / 2 - ws->geometry().width() / 2);
    startY = int (this->geometry().y() + this->geometry().height() / 2 - ws->geometry().height() /2);
    ws->setGeometry(startX, startY, ws->geometry().width(), ws->geometry().height());
    ws->setWindowModality(Qt::WindowModal);
    ws->setAttribute(Qt::WA_DeleteOnClose);
    ws->show();
}

void MainWindow::actionPlayTriggered(bool checked)
{
    this->youtube->startDownload();
}

void MainWindow::on_actionClipboard_Watchdog_toggled(bool checked)
{
    if(checked)
    {
        connect(this->clipboard, SIGNAL(dataChanged()), this, SLOT(onClipboardChanged()));
    } else
    {
        disconnect(this->clipboard, SIGNAL(dataChanged()), this, SLOT(onClipboardChanged()));
    }
}

void MainWindow::onClipboardChanged()
{
    QStringList formats = this->clipboard->mimeData(QClipboard::Clipboard)->formats();
    QString text;
    if(formats.contains("text/html") || formats.contains("text/plain"))
    {
        QString type = formats.contains("text/html")?"text/html":"text/plain";
        text = QString(this->clipboard->mimeData(QClipboard::Clipboard)->data(type));
        if(this->lastClipboard == text)
        {
            return;
        }
        this->lastClipboard = text;
        this->youtube->clipboardChanged(text);
    }
}

void MainWindow::on_actionAnalyse_clipboard_for_links_triggered()
{
    this->onClipboardChanged();
}
