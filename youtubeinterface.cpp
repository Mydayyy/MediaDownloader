#include "youtubeinterface.h"

YoutubeInterface::YoutubeInterface()
{

}

// METHODS
void YoutubeInterface::extractVideoInformation(QString url)
{
    Process *extractorProcess = new Process(this);
    connect(extractorProcess, SIGNAL(finished(int)), this, SLOT(videoExtractionProcessEnd(int)));
    connect(extractorProcess, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(videoExtractionProcessErrorOccured(QProcess::ProcessError)));
    extractorProcess->start("youtube-dl -s -j " + url);

}

void YoutubeInterface::downloadVideo(Link *link)
{
    qDebug() << QString().sprintf("%08p", link) << " Download called";
    Process *downloaderProcess = new Process(this);
    downloaderProcess->setLink(link);
    connect(downloaderProcess, SIGNAL(finished(int)), this, SLOT(videoDownloadingProcessEnd(int)));
    connect(downloaderProcess, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(videoDownloadingProcessErrorOccured(QProcess::ProcessError)));
    connect(downloaderProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(videoDownloadingProcessStdOut()));
    connect(downloaderProcess, SIGNAL(readyReadStandardError()), this, SLOT(videoDownloadingProcessStdErr()));
    QString destinationPath = SettingsManager::getInstance().get("downloadSavePath").toString();

    // TODO: POSSIBLE INFINITE LOOP - ERROR HANDLING
    qDebug() << QString().sprintf("%08p", link) << " Starting to extract name";
    QEventLoop loop;
    Process *getFilenameProcess = new Process(this);
    connect(getFilenameProcess, SIGNAL(finished(int)), &loop, SLOT(quit()));
    getFilenameProcess->start("youtube-dl --no-mtime --get-filename -f best -o \"./downloads/%(title)s.%(ext)s\" " + link->getData(Link::DATA_LINK).toString() + "");
    loop.exec();
    destinationPath = getFilenameProcess->readAllStandardOutput();
    destinationPath = destinationPath.simplified();
    qDebug() << QString().sprintf("%08p", link) << " Name extraction finished";
    QFileInfo fileCheck(destinationPath);
    QFileInfo partFile(destinationPath+".part"); // TODO: Handle resume download when partfile present

    QString skipOption = "";
    if(this->isDialogOpen) {
        qDebug() << QString().sprintf("%08p", link) << " Dialog open, aborting";
        downloaderProcess->deleteLater();
        return;
    }
    if(fileCheck.exists() && fileCheck.isFile() || partFile.exists() && partFile.isFile() || this->createdFilepaths.contains(destinationPath))
    {
        qDebug() << QString().sprintf("%08p", link) << " File exists. Preparing dialog";
        QMessageBox mb;
        QCheckBox *cb = new QCheckBox("Do you want to apply this behaviour to all future incidents "\
                                      "occuring in the current download session?");
        bool applyBehaviour = false;

        mb.setText("The File LOREMIPSUM already exists. How to you want to proceed?");
        mb.setIcon(QMessageBox::Icon::Question);
        mb.setWindowFlags(mb.windowFlags() & ~Qt::WindowCloseButtonHint);
        QAbstractButton *renameButton = (QAbstractButton*) mb.addButton("Rename", QMessageBox::ActionRole);
        QAbstractButton *skipButton = (QAbstractButton*) mb.addButton("Skip", QMessageBox::RejectRole);
        QAbstractButton *overwriteButton = (QAbstractButton*) mb.addButton("Overwrite", QMessageBox::DestructiveRole);
        mb.setCheckBox(cb);

        QObject::connect(cb, &QCheckBox::stateChanged, [&applyBehaviour](int state){
            if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked)
            {
                applyBehaviour = true;
            }
            else if(static_cast<Qt::CheckState>(state) == Qt::CheckState::Unchecked)
            {
                applyBehaviour = false;
            }
        });

        if(this->overwriteBehaviour == OverwriteBehaviour::NONE)
        {
            this->isDialogOpen = true;
            qDebug() << QString().sprintf("%08p", link) << " Opening dialog";
            mb.exec();
            qDebug() << QString().sprintf("%08p", link) << " Closing dialog";
            this->isDialogOpen = false;
        }

        if(mb.clickedButton() == renameButton || this->overwriteBehaviour == OverwriteBehaviour::RENAME) // RENAME
        {
            if(applyBehaviour)
            {
                this->overwriteBehaviour = OverwriteBehaviour::RENAME;
            }
            QFileInfo newFile = this->makeFilepathUnique(destinationPath);
            destinationPath = newFile.path() + QDir::separator() + newFile.baseName() + "." + newFile.completeSuffix();
            link->setData(Link::DATA_TITLE, newFile.baseName());
            emit downloadVideoRenamed(link, newFile.baseName());
        }
        if(mb.clickedButton() == skipButton || this->overwriteBehaviour == OverwriteBehaviour::SKIP) // SKIP
        {
            if(applyBehaviour)
            {
                this->overwriteBehaviour = OverwriteBehaviour::SKIP;
            }
            emit downloadVideoSkipped(link);
            return;
        }
        if(mb.clickedButton() == overwriteButton || this->overwriteBehaviour == OverwriteBehaviour::OVERWRITE) // OVERWRITE
        {
            if(applyBehaviour)
            {
                this->overwriteBehaviour = OverwriteBehaviour::OVERWRITE;
            }
            skipOption = "--no-continue ";
        }
        cb->deleteLater();
    }
    qDebug() << QString().sprintf("%08p", link) << " Starting youtube-dl";
    this->createdFilepaths.append(destinationPath);
    downloaderProcess->start("youtube-dl --no-mtime "+skipOption+"-f best -o \""+destinationPath+"\" " + link->getData(Link::DATA_LINK).toString() + "");
    emit downloadVideoStarted(link);
}

QFileInfo YoutubeInterface::makeFilepathUnique(QString filepath)
{
    QFileInfo originalFile(filepath);
    QFileInfo newFile(filepath);
    QString newFilepath = filepath;

    // TODO: HANDLE INFINIT LOOP
    int i = 1;
    while(newFile.exists() || this->createdFilepaths.contains(newFilepath))
    {
        newFilepath = originalFile.path() + QDir::separator() + originalFile.baseName() + " " + QString::number(i) +"." + originalFile.completeSuffix();
        newFile = QFileInfo(newFilepath);
        i++;
    }
    this->createdFilepaths.append(newFilepath);
    return newFile;
}

// SLOTS

// Process Video Downloading Slots
void YoutubeInterface::videoDownloadingProcessEnd(int exitCode)
{
    Process *downloadProcess = (Process*) this->sender();
    downloadProcess->deleteLater();
    emit downloadVideoFinished(downloadProcess->getLink());
}

void YoutubeInterface::videoDownloadingProcessErrorOccured(Process::ProcessError error)
{
    Process *downloadProcess = (Process*) this->sender();
    downloadProcess->deleteLater();
    emit downloadVideoFailed(downloadProcess->getLink(), "Could not download video. Error code: " + QString::number(error));
    emit downloadVideoFinished(downloadProcess->getLink());
}

void YoutubeInterface::videoDownloadingProcessStdOut()
{
    Process *downloadProcess = (Process*) this->sender();
    Link *link = downloadProcess->getLink();
    QString content = downloadProcess->readAllStandardOutput();
    QRegularExpression progressInfo("\\[download\\]\\s+([0-9]+\\.[0-9]+%)\\s+of\\s+([0-9]+\\.[0-9]+MiB)\\s+at\\s+([0-9]+\\.[0-9]+MiB\\/s)\\s+ETA\\s+([0-9]+:[0-9]+)");
    QRegularExpressionMatch m = progressInfo.match(content);
    if (m.hasMatch()) {
        QString percentage = m.captured(1);
        QString maxsize = m.captured(2);
        QString speed = m.captured(3);
        QString remaining = m.captured(3);
        emit downloadVideoUpdateProgress(link, percentage, maxsize, speed, remaining);
        return;
    }
    //\[download\] 100% of ([0-9]+\.[0-9]+MiB)(?:\sin\s([0-9]+:[0-9]+))?

    QRegularExpression finishinfo("\\[download\\]\\s100%\\sof\\s([0-9]+\\.[0-9]+MiB)(?:\\sin\\s([0-9]+:[0-9]+))?");
    m = finishinfo.match(content);
    if (m.hasMatch()) {
        QString maxsize;
        QString time;
        if(m.lastCapturedIndex() == 1) {
            maxsize = m.captured(1);
        } else {
            maxsize = m.captured(1);
            time = m.captured(2);
        }
        emit downloadVideoUpdateProgressLast(link, maxsize, time);
        return;
    }
}

void YoutubeInterface::videoDownloadingProcessStdErr()
{
    Process *downloadProcess = (Process*) this->sender();
    QString a = downloadProcess->readAllStandardError();
    emit downloadVideoFailed(downloadProcess->getLink(), a);
    qDebug() << QString().sprintf("%08p", downloadProcess->getLink()) << " " << a;
}

// Process Video Extraction Slots
void YoutubeInterface::videoExtractionProcessEnd(int exitCode)
{
    Q_UNUSED(exitCode);

    Process *extractorProcess = (Process*) this->sender();
    extractorProcess->deleteLater();

    QByteArray stdout = extractorProcess->readAllStandardOutput();
    QByteArray stderr = extractorProcess->readAllStandardError();
    if(stderr.size() > 0) // Seems like we ran into an error.
    {
        emit extractedVideoInformationFailed(stderr);
        return;
    }
    if(stdout.size() == 0) // Not quite sure whether this can happen.
    {
        emit extractedVideoInformationFailed("We could not find any videos");
        return;
    }
    // Do not read further, thanks!
    QString videoInformation = stdout;
    videoInformation = videoInformation.replace('\n', "");
    videoInformation = videoInformation.replace("}{", "},{");
    videoInformation = "{\"videos\":[" + videoInformation + "]}";

    QJsonDocument doc = QJsonDocument::fromJson(videoInformation.toUtf8());
    if(doc.isNull())
    {
        emit extractedVideoInformationFailed(stderr);
        return;
    }
    QJsonObject root = doc.object();
    QJsonArray videos = root["videos"].toArray();
    int videoCount = videos.count();

    QList<Link*> videoLinks;
    QString playlistTitle = "";

    for(int i = 0; i < videoCount; i++)
    {
        QJsonObject video = videos.at(i).toObject();
        QString title = video["title"].toString();
        QString url = video["webpage_url"].toString();
        Link *link = new Link(title, url, "0%");
        videoLinks.append(link);

        if(video.contains("playlist_title"))
        {
            QString videoPlaylistTitle = video["playlist_title"].toString();
            if(!videoPlaylistTitle.isEmpty() && playlistTitle.isEmpty())
            {
                playlistTitle = videoPlaylistTitle;
            }
        }
    }
    emit extractedVideoInformation(videoLinks, playlistTitle);
}

void YoutubeInterface::videoExtractionProcessErrorOccured(Process::ProcessError error)
{
    Process *extractorProcess = (Process*) this->sender();
    extractorProcess->deleteLater();
    emit extractedVideoInformationFailed("Extraction Process could not be started. Error Code: " + QString::number(error));
}
