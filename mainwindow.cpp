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
    QAction *actionPlay = ui->toolBar->addAction(QIcon(":/toolbar/icons/play.png"), tr("Pause"));
    connect(actionPlay, SIGNAL(triggered(bool)), this, SLOT(actionPlayTriggered(bool)));

    // Unused for now. TODO: Implement
    ui->toolBar->addAction(QIcon(":/toolbar/icons/pause.png"), tr("Pause"));
    ui->toolBar->addAction(QIcon(":/toolbar/icons/stop.png"), tr("Pause"));

    this->setupTableModel();
    this->youtube = new YoutubeDownloader(this->tableModel);
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

    connect(ui->treeTrackView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableViewCustomContextMenu(QPoint))); // Custom Context Menu
}

void MainWindow::tableViewCustomContextMenu(QPoint pos)
{
    // TODO: We ( usually ) want context actions to apply to all selected items, so we need to iterate over them here and apply the action to each of them
    QMenu menu(this);

    QAction *deleteRow = new QAction("Delete", &menu); // Deletes an entry
    menu.addAction(deleteRow);

    QAction *selectedAction = menu.exec(ui->treeTrackView->viewport()->mapToGlobal(pos));

    if(!ui->treeTrackView->indexAt(pos).isValid())
    {
        return;
    }
    if(selectedAction == deleteRow)
    {
        QModelIndex selectedIndex = ui->treeTrackView->indexAt(pos);
        this->tableModel->deleteIndexRow(selectedIndex);
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
