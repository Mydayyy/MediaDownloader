#include "youtubedownloader.h"

YoutubeDownloader::YoutubeDownloader(TableModel *tableModel, QObject *parent)
    : QObject(parent),
      tableModel(tableModel)
{
    this->ytd = new YoutubeInterface();
    connect(this->ytd, SIGNAL(extractedVideoInformation(QList<Link*>,QString)),
            this, SLOT(extractedLinkInformation(QList<Link*>,QString)));
    connect(this->ytd, SIGNAL(extractedVideoInformationFailed(QString)), this, SLOT(extractLinkInformationFailed(QString)));

    connect(this->ytd, SIGNAL(downloadVideoFailed(Link*,QString)), this, SLOT(downloadVideoFailed(Link*,QString)));
    connect(this->ytd, SIGNAL(downloadVideoSkipped(Link*)), this, SLOT(downloadVideoSkipped(Link*)));
    connect(this->ytd, SIGNAL(downloadVideoRenamed(Link*,QString)), this, SLOT(downloadVideoRenamed(Link*,QString)));
    connect(this->ytd, SIGNAL(downloadVideoUpdateProgress(Link*,QString,QString,QString,QString)),
            this, SLOT(downloadVideoUpdateProgress(Link*,QString,QString,QString,QString)));
    connect(this->ytd, SIGNAL(downloadVideoUpdateProgressLast(Link*,QString,QString)),
            this, SLOT(downloadVideoUpdateProgressLast(Link*,QString,QString)));
    connect(this->ytd, SIGNAL(downloadVideoFinished(Link*)), this, SLOT(downloadVideoFinished(Link*)));
    connect(this->ytd, SIGNAL(dialogClosed()), this, SLOT(dialogClosed()));

    connect(&timerDownloadWatchdog, SIGNAL(timeout()), this, SLOT(downloadWatchdog()));
}

YoutubeDownloader::~YoutubeDownloader()
{
    delete this->ytd;
}

void YoutubeDownloader::addLink(QString url)
{
    this->extractLinkInformation(url);
}

void YoutubeDownloader::startDownload()
{
    if(this->isDownloading)
    {
        return;
    }
    this->isDownloading = true;
    this->timerDownloadWatchdog.start(1000);
    emit this->startedOperating();
    this->downloadNext();
}



void YoutubeDownloader::extractLinkInformation(QString url)
{
    this->pendingExtractionProcesses++;
    this->ytd->extractVideoInformation(url);
}

void YoutubeDownloader::extractedLinkInformation(QList<Link *> videos, QString playlistTitle)
{
    this->pendingExtractionProcesses--;
    if(videos.length() == 1)
    {
        this->tableModel->addLink(videos[0]);
    }
    else
    {
        QString containerName;
        if(!playlistTitle.isEmpty())
        {
            containerName = playlistTitle;
        }
        else
        {
            containerName = videos.at(0)->getData(Link::DATA_TITLE).toString();
        }
        Link *container = new Link(containerName, "", "");
        container->setData(Link::DATA_IS_CONTAINER, true);
        this->tableModel->addLink(container);

        for(int i = 0; i < videos.length(); i++)
        {
            this->tableModel->addLink(videos[i], container);
        }
    }
}

void YoutubeDownloader::extractLinkInformationFailed(QString stderr)
{
    this->pendingExtractionProcesses--;
    QMessageBox mb("Error",
                   QString(stderr),
                   QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    mb.exec();
}

void YoutubeDownloader::downloadNext()
{
    if(!this->isDownloading || this->ytd->isDialogOpen)
    {
        return;
    }
    for(int i = 0; i < SettingsManager::getInstance().get("concurrentDownloads", DEFAULT_CONCURRENT_DOWNLOADS).toInt(); i++)
    {
        Link *link = this->tableModel->getUnprocessedLink();
        if(!link)
        {
            if(this->pendingDownloadProcesses == 0)
            {
                this->isDownloading = false;
                this->timerDownloadWatchdog.stop();
                emit stoppedOperating();
            }
            return;
        }
        if(this->pendingDownloadProcesses >= SettingsManager::getInstance().get("concurrentDownloads", DEFAULT_CONCURRENT_DOWNLOADS).toInt())
        {
            return;
        }
        this->pendingDownloadProcesses++;
        link->setData(Link::DATA_IS_STARTED, QVariant(true));
        this->ytd->downloadVideo(link);
    }
}

void YoutubeDownloader::downloadVideoFailed(Link *link, QString error)
{
    // DON'T START NEXT, VIDEO FINISHED WILL STILL BE EMITTED
    QMessageBox mb("Error",
                   QString(error),
                   QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    mb.exec();
}

void YoutubeDownloader::downloadVideoSkipped(Link *link)
{
    this->pendingDownloadProcesses--;
    this->tableModel->updateLinkProgress(link, "Skipped");
    this->downloadNext();
}

void YoutubeDownloader::downloadVideoRenamed(Link *link, QString newName)
{
    this->tableModel->refreshName(link);
}

void YoutubeDownloader::downloadVideoUpdateProgress(Link *link, QString percentage, QString maxsize, QString speed, QString remaining)
{
    QString progress = ""+percentage+" of "+maxsize+" at "+speed+" ( "+remaining+" )";
    this->tableModel->updateLinkProgress(link, progress);
}

void YoutubeDownloader::downloadVideoUpdateProgressLast(Link *link, QString maxsize, QString time)
{
    QString progress = "100% of " + maxsize;
    if(!time.isEmpty()) {
        progress += "  in " + time;
    }
    progress += " ( FINISHED )";
    this->tableModel->updateLinkProgress(link, progress);
}

void YoutubeDownloader::downloadVideoFinished(Link *link)
{
    this->pendingDownloadProcesses--;
    link->setData(Link::DATA_IS_FINISHED, true);
    this->downloadNext();
}

void YoutubeDownloader::downloadWatchdog()
{
    this->downloadNext();
}

void YoutubeDownloader::dialogClosed()
{
    qDebug() << "Downloader dialog Closed";
    this->downloadNext();
}
