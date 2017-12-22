#include "youtubedownloader.h"

YoutubeDownloader::YoutubeDownloader(TableModel *tableModel, QObject *parent)
    : QObject(parent),
      tableModel(tableModel)
{
    this->ytd = new YoutubeInterface();

    // Video Information Extraction
    connect(this->ytd, SIGNAL(extractedVideoInformation(QList<MediaObject*>,QString)),
            this, SLOT(extractedLinkInformation(QList<MediaObject*>,QString)));
    connect(this->ytd, SIGNAL(extractedVideoInformationFailed(QString, bool)), this, SLOT(extractLinkInformationFailed(QString, bool)));

    // Video Download Extraction
    connect(this->ytd, SIGNAL(downloadVideoFailed(MediaObject*,QString)), this, SLOT(downloadVideoFailed(MediaObject*,QString)));
    connect(this->ytd, SIGNAL(downloadVideoSkipped(MediaObject*)), this, SLOT(downloadVideoSkipped(MediaObject*)));
    connect(this->ytd, SIGNAL(downloadVideoRenamed(MediaObject*,QString)), this, SLOT(downloadVideoRenamed(MediaObject*,QString)));
    connect(this->ytd, SIGNAL(downloadVideoUpdateProgress(MediaObject*,QString,QString,QString,QString)),
            this, SLOT(downloadVideoUpdateProgress(MediaObject*,QString,QString,QString,QString)));
    connect(this->ytd, SIGNAL(downloadVideoDownloadFinished(MediaObject*,QString,QString)),
            this, SLOT(downloadVideoDownloadFinished(MediaObject*,QString,QString)));
    connect(this->ytd, SIGNAL(downloadVideoFinished(MediaObject*)), this, SLOT(downloadVideoFinished(MediaObject*)));

    // Setup a timer which will try to start new downloads at a specific interval.
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



void YoutubeDownloader::extractLinkInformation(QString url, bool reportErrors)
{
    this->pendingExtractionProcesses++;
    this->ytd->extractVideoInformation(url, reportErrors); // Will report success / error via connected slot
}

void YoutubeDownloader::clipboardChanged(QString newText)
{
    // Taken from https://mathiasbynens.be/demo/url-regex
    QString UrlRegex = "(?:(?:https?|ftp):\\/\\/)(?:\\S+(?::\\S*)?@)?(?:(?!10(?:\\.\\d{1,3}){3})(?!127(?:\\.\\d{1,3}){3})(?!169\\.254(?:\\.\\d{1,3}){2})(?!192\\.168(?:\\.\\d{1,3}){2})(?!172\\.(?:1[6-9]|2\\d|3[0-1])(?:\\.\\d{1,3}){2})(?:[1-9]\\d?|1\\d\\d|2[01]\\d|22[0-3])(?:\\.(?:1?\\d{1,2}|2[0-4]\\d|25[0-5])){2}(?:\\.(?:[1-9]\\d?|1\\d\\d|2[0-4]\\d|25[0-4]))|(?:(?:[a-z\\x{00a1}-\\x{ffff}0-9]+-?)*[a-z\\x{00a1}-\\x{ffff}0-9]+)(?:\\.(?:[a-z\\x{00a1}-\\x{ffff}0-9]+-?)*[a-z\\x{00a1}-\\x{ffff}0-9]+)*(?:\\.(?:[a-z\\x{00a1}-\\x{ffff}]{2,})))(?::\\d{2,5})?(?:\\/[^\\s]*)?";
    QRegularExpression exp(UrlRegex);
    QRegularExpressionMatchIterator i = exp.globalMatch(newText);
    while(i.hasNext())
    {
        QRegularExpressionMatch urlMatch = i.next();
        QString url = urlMatch.captured(0);
        QRegularExpression regex("[!\"\\\\]");
        while(regex.match(url).hasMatch())
        {
            url.chop(1);
        }
        this->extractLinkInformation(url, false);
    }
}

void YoutubeDownloader::extractedLinkInformation(QList<MediaObject *> videos, QString playlistTitle)
{
    this->pendingExtractionProcesses--;
    if(videos.length() == 1) // This means we did not encounter a playlist, so we just append the song to the table
    {
        this->tableModel->addLink(videos[0]);
    }
    else // If it is more than one video, we are going to fill it in a container
    {
        QString containerName;
        if(!playlistTitle.isEmpty()) // Check whether youtube-dl provided us with a playlist name
        {
            containerName = playlistTitle;
        }
        else // otherwise we just take the name of the first video as the container name
        {
            containerName = videos.at(0)->getData(MediaObject::DATA_TITLE).toString();
        }
        MediaObject *container = new MediaObject(containerName, "", "");
        container->setData(MediaObject::DATA_IS_CONTAINER, true); // We need to set the entry as a container
        this->tableModel->addLink(container);

        for(int i = 0; i < videos.length(); i++) // And add all videos to the container
        {
            this->tableModel->addLink(videos[i], container);
        }
    }
}

void YoutubeDownloader::extractLinkInformationFailed(QString sterr, bool reportError)
{
    this->pendingExtractionProcesses--;
    if(!reportError)
    {
        return;
    }
    QMessageBox mb("Error",
                   QString(sterr),
                   QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    mb.exec();
}

void YoutubeDownloader::downloadNext()
{
    // When a dialog is opened, asking the user what to do in a case of a duplicate,
    // we do not want to download any further videos but instead wait for his response
    if(this->isDownloading == false || this->ytd->isDialogOpen)
    {
        return;
    }
    for(int i = 0; i < SettingsManager::getInstance().get("concurrentDownloads", DEFAULT_CONCURRENT_DOWNLOADS).toInt(); i++) //
    {
        MediaObject *link = this->tableModel->getUnprocessedLink(); // Get the next unprocessed link
        if(!link) // When no link was returned we can return here. In case we also have no pending download processes, we're also exiting download mode
        {
            if(this->pendingDownloadProcesses == 0)
            {
                this->isDownloading = false;
                this->timerDownloadWatchdog.stop();
                this->ytd->resetDownloadSession();
                emit stoppedOperating();
            }
            return;
        }
        // When we already have all download slots busy, we return here
        // We might want to consider checking this at the start of this function. No clue why it is here.
        if(this->pendingDownloadProcesses >= SettingsManager::getInstance().get("concurrentDownloads", DEFAULT_CONCURRENT_DOWNLOADS).toInt())
        {
            return;
        }
        this->pendingDownloadProcesses++;
        link->setData(MediaObject::DATA_IS_STARTED, QVariant(true));
        if(SettingsManager::getInstance().get("createContainerSubfolder", DEFAULT_CREATE_CONTAINER_SUBFOLDER).toBool()) // Check whether we need to create a subfolder for containers
        {
            MediaObject *parentLink = this->tableModel->getParentLink(link); // Get the parent
            if(parentLink) // When it is valid, we take the name of the parent ( the container ) and use it as the subfolder name
            {
                this->ytd->downloadVideo(link, parentLink->getData(MediaObject::DATA_TITLE).toString());
            }
            else
            {
                this->ytd->downloadVideo(link); // Otherwise just download it to the root
            }
        }
        else
        {
            this->ytd->downloadVideo(link);
        }

    }
}

void YoutubeDownloader::downloadVideoFailed(MediaObject *link, QString error)
{
    // VIDEO FINISHED WILL STILL BE EMITTED
    QMessageBox mb("Error",
                   QString(error),
                   QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    mb.exec();
    link->setData(MediaObject::DATA_IS_FAILED, QVariant(true));
}

void YoutubeDownloader::downloadVideoSkipped(MediaObject *link)
{
    this->pendingDownloadProcesses--;
    link->setData(MediaObject::DATA_IS_SKIPPED, true);
}

void YoutubeDownloader::downloadVideoRenamed(MediaObject *link, QString newName)
{
    // The video was renamed due to being a duplicate. Just ensure that the table model also represents the naming change
    this->tableModel->refreshName(link);
}

void YoutubeDownloader::downloadVideoUpdateProgress(MediaObject *link, QString percentage, QString maxsize, QString speed, QString remaining)
{
    link->setData(MediaObject::DATA_DOWNLOADED_SIZE, percentage);
    link->setData(MediaObject::DATA_MAX_SIZE, maxsize);
    link->setData(MediaObject::DATA_SPEED, speed);
    link->setData(MediaObject::DATA_TIME, remaining);
    this->tableModel->updateLinkProgress(link);
}

void YoutubeDownloader::downloadVideoDownloadFinished(MediaObject *link, QString maxsize, QString time)
{
    link->setData(MediaObject::DATA_TIME, time);
    link->setData(MediaObject::DATA_MAX_SIZE, maxsize);
    this->tableModel->updateLinkProgress(link);
}

void YoutubeDownloader::downloadVideoFinished(MediaObject *link)
{
    link->setData(MediaObject::DATA_IS_FINISHED, true);
    this->tableModel->updateLinkProgress(link);
    this->pendingDownloadProcesses--;
}

void YoutubeDownloader::downloadWatchdog()
{
    this->downloadNext();
}
