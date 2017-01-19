#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QDir::setCurrent(QCoreApplication::applicationDirPath());

    QAction *actionPlay = ui->toolBar->addAction(QIcon(":/toolbar/icons/play.png"), tr("Pause"));
    connect(actionPlay, SIGNAL(triggered(bool)), this, SLOT(actionPlayTriggered(bool)));

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
    this->ui->treeTrackView->header()->setSectionResizeMode(QHeaderView::Stretch);
    this->ui->treeTrackView->setModel(this->tableModel);

    ui->treeTrackView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeTrackView->setDragEnabled(true);
    ui->treeTrackView->viewport()->setAcceptDrops(true);
    ui->treeTrackView->setDropIndicatorShown(true);
    ui->treeTrackView->setDragDropMode(QAbstractItemView::DragDrop);
    ui->treeTrackView->setDefaultDropAction(Qt::MoveAction);
    ui->treeTrackView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeTrackView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableViewCustomContextMenu(QPoint)));
}

void MainWindow::tableViewCustomContextMenu(QPoint pos)
{
    QMenu menu(this);

    QAction *deleteRow = new QAction("Delete", &menu);
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
