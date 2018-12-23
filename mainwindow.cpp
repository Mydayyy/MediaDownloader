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

    QAction *actionStop = ui->toolBar->addAction(QIcon(":/toolbar/icons/stop.png"), tr("Stop"));
    connect(actionStop, SIGNAL(triggered(bool)), this, SLOT(actionStopTriggered(bool)));

    // Unused for now. TODO: Implement
    //ui->toolBar->addAction(QIcon(":/toolbar/icons/pause.png"), tr("Pause"));


    this->setupTableModel();
    this->mediaDownloader = new MediaDownloader(this->tableModel);
    this->clipboard = QApplication::clipboard();

    updateSettingPanel();

//    this->tableModel->addLink("Test1");
//    this->tableModel->addLink("Test2");
//    auto a = this->tableModel->addLink("Test1");
//    this->tableModel->addLink("Test2", a);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete this->mediaDownloader;
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

//    this->tableModel->addLink("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
//    MediaObject *parentLink1 = this->tableModel->addLink("Link1");
//        this->tableModel->addLink("Link1.1", parentLink1);
//    MediaObject *parentLink11 = this->tableModel->addLink("Link1.2", parentLink1);
//            this->tableModel->addLink("Link1.2.1", parentLink11);
//            this->tableModel->addLink("Link1.2.2", parentLink11);
//            this->tableModel->addLink("Link1.2.3", parentLink11);
//        this->tableModel->addLink("Link1.3", parentLink1);
//    this->tableModel->addLink("Link2");
//    this->tableModel->addLink("Link3");
//    this->tableModel->addLink("Link4");

    connect(ui->treeTrackView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableViewCustomContextMenu(QPoint))); // Custom Context Menu
    connect(ui->treeTrackView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
}

void MainWindow::tableViewCustomContextMenu(QPoint pos)
{
    // TODO: We ( usually ) want context actions to apply to all selected items, so we need to iterate over them here and apply the action to each of them
    if(!ui->treeTrackView->indexAt(pos).isValid())
    {
        return;
    }
    QModelIndex selectedIndex = ui->treeTrackView->indexAt(pos);
    MediaObject *link = static_cast<TreeNode*>(selectedIndex.internalPointer())->getLink();

    qDebug() << link->getData(MediaObject::DATA_TITLE);

    QMenu menu(this);

    QAction *deleteRow = new QAction("Delete", &menu); // Deletes an entry
    menu.addAction(deleteRow);

    QAction *retryLink = new QAction("Retry", &menu);
    if(link->getData(MediaObject::DATA_IS_FAILED).toBool())
    {
        menu.addAction(retryLink);
    }

    QAction *renameContainer = new QAction("Rename", &menu);
    if(link->getData(MediaObject::DATA_IS_CONTAINER).toBool())
    {
        menu.addAction(renameContainer);
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
    if(selectedAction == renameContainer)
    {
        bool ok;
        QString newName = QInputDialog::getText(this, "Rename Container", "Enter a new name:", QLineEdit::Normal, QString(), &ok);
        if(ok && !newName.isEmpty())
        {
            link->setData(MediaObject::DATA_TITLE, QVariant(newName));
        }
    }
}



void MainWindow::on_buttonAddTrack_clicked()
{
    this->mediaDownloader->addLink(ui->inputUrl->text());
}

void MainWindow::on_actionSettings_triggered()
{
    WindowSettings *ws = new WindowSettings(this);
    connect(ws, &WindowSettings::destroyed, [=](QObject *obj){
       this->updateSettingPanel();
    });
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
    this->mediaDownloader->startDownload();
}

void MainWindow::actionStopTriggered(bool checked)
{
    this->mediaDownloader->stopDownload();
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
        this->mediaDownloader->clipboardChanged(text);
    }
}

void MainWindow::on_actionAnalyse_clipboard_for_links_triggered()
{
    this->onClipboardChanged();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "MediaDownloader", "<a href='https://github.com/mydayyy'>https://github.com/mydayyy</a>");
}

// INDIVIDUAL SONG SETTINGS STARTING HERE

void MainWindow::onSelectionChanged(const QItemSelection &deselected, const QItemSelection &selected)
{
    this->updateSettingPanel();
}

void MainWindow::setSettingForCurrentObjects(QString key, QVariant val) {
    QModelIndexList selectedIndexes = this->ui->treeTrackView->selectionModel()->selectedIndexes();
    QList<TreeNode*> treeNodes = this->tableModel->convertIndexListToTreeNodeList(selectedIndexes);

    for(auto it = treeNodes.begin(); it != treeNodes.end(); it++) {
        (*it)->getLink()->getSettings()->set(key, val);
        this->tableModel->redrawMediaObject((*it)->getLink());
    }
}

void MainWindow::updateSettingPanel()
{
    QModelIndexList selectedIndexes = this->ui->treeTrackView->selectionModel()->selectedIndexes();
    QList<TreeNode*> treeNodes = this->tableModel->convertIndexListToTreeNodeList(selectedIndexes);

    if(treeNodes.size() != 1) {
        ui->settingsPanel->hide();
        return;
    }
    ui->settingsPanel->show();

    TreeNode *tn = treeNodes.at(0);

    ui->inputSavePath->setText(tn->getSettingsValue("downloadSavePath").toString());

    QString format = tn->getSettingsValue("audioFormat").toString();
    int index = ui->audioFormat->findText(format);
    ui->audioFormat->setCurrentIndex(index);

    ui->extractOnlyAudio->setChecked(tn->getSettingsValue("extractOnlyAudio").toBool());
}

void MainWindow::on_buttonBrowseFiles_clicked()
{
    QFileDialog fileBrowser(this, "Choose a download location", Settings::getInstance().get("downloadSavePath").toString());
    fileBrowser.setFileMode(QFileDialog::DirectoryOnly);
    if(fileBrowser.exec() == QDialog::Accepted)
    {
        if(fileBrowser.selectedFiles().size() > 0)
        {
            QString savePath = fileBrowser.selectedFiles()[0];
            setSettingForCurrentObjects("downloadSavePath", savePath);
            ui->inputSavePath->setText(savePath);
        }
    }
}

void MainWindow::on_inputSavePath_textEdited(const QString &text)
{
    setSettingForCurrentObjects("downloadSavePath", this->ui->inputSavePath->text());
}

void MainWindow::on_extractOnlyAudio_toggled(bool checked)
{
    setSettingForCurrentObjects("extractOnlyAudio", QVariant(checked));
}

void MainWindow::on_audioFormat_currentTextChanged(const QString &arg1)
{
    setSettingForCurrentObjects("audioFormat", QVariant(arg1));
}
