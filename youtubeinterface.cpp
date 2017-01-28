#include "youtubeinterface.h"

YoutubeInterface::YoutubeInterface()
{

}

/
void YoutubeInterface::extractVideoInformation(QString url)
{
    Process *extractorProcess = new Process(this);
    connect(extractorProcess, SIGNAL(finished(int)), this, SLOT(videoExtractionProcessEnd(int))); // on Process Finished
    connect(extractorProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), // on Process Startup Error
            this, SLOT(videoExtractionProcessErrorOccured(QProcess::ProcessError)));
    extractorProcess->start("youtube-dl --ignore-errors -s -j " + url); // -j  will dump the json, -s will simulate and not actually download anything

}

void YoutubeInterface::downloadVideo(Link *link, QString containerTitle)
{
    DownloadData data;
    data.fullpath = "";
    data.name = "";
    data.containerTitle = containerTitle;
    this->runningDownloads[link] = data; // Add to running downloads

    QDir path(SettingsManager::getInstance().get("downloadSavePath", DEFAULT_DOWNLOAD_SAVE_PATH).toString());

    if(!containerTitle.isEmpty()) // In case the subfolder does not exist, create it
    {
        path.mkdir(containerTitle);
        path.cd(containerTitle);
    }

    QFileInfo destinationFile = path.path() + QDir::separator() + "%(title)s.%(ext)s";
    QString destinationPath = destinationFile.path() + QDir::separator() + destinationFile.baseName() + "." + destinationFile.completeSuffix();
    // TODO: POSSIBLE STUCK HERE - ADD ERROR HANDLING
    Process *getFilenameProcess = new Process(this);
    getFilenameProcess->setLink(link); // We need to access the link later in the slots
    connect(getFilenameProcess, SIGNAL(finished(int)), this, SLOT(extractedFilename(int)));
    getFilenameProcess->start("youtube-dl --no-mtime --get-filename -f best -o \""+destinationPath+"\" " + link->getData(Link::DATA_LINK).toString() + "");
}

void YoutubeInterface::resetDownloadSession()
{
    this->createdFilepaths.clear();
    this->overwriteBehaviour = OverwriteBehaviour::NONE;
}

void YoutubeInterface::extractedFilename(int exitcode)
{
    Q_UNUSED(exitcode);

    Process *getFilenameProcess = (Process*) this->sender();
    getFilenameProcess->deleteLater();
    Link *link = getFilenameProcess->getLink();

    Process *downloaderProcess = new Process(this);
    downloaderProcess->setLink(link);
    connect(downloaderProcess, SIGNAL(finished(int)), this, SLOT(videoDownloadingProcessEnd(int)));
    connect(downloaderProcess, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(videoDownloadingProcessErrorOccured(QProcess::ProcessError)));
    connect(downloaderProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(videoDownloadingProcessStdOut()));
    connect(downloaderProcess, SIGNAL(readyReadStandardError()), this, SLOT(videoDownloadingProcessStdErr()));
    QString destinationPath = SettingsManager::getInstance().get("downloadSavePath").toString();

    destinationPath = getFilenameProcess->readAllStandardOutput(); // Youtube-dl prints the name of the video in a single line
    destinationPath = destinationPath.simplified(); // Strip it of \n\r\t and multiple spaces...
    QFileInfo fileCheck(destinationPath);
    QFileInfo partFile(destinationPath+".part"); // TODO: Handle resume download when partfile present
    this->runningDownloads[link].fullpath = destinationPath;

    // When the dialog is open, we abort all downloads until the user made a decision. They will be resumed after.
    if(this->isDialogOpen) {
        downloaderProcess->deleteLater();
        this->downloadsToResume.append(getFilenameProcess->getLink());
        this->runningDownloads.remove(link);
        return;
    }

    QString skipOption = ""; // Used to overwrite the file in case the user made the decision.
    if((fileCheck.exists() && fileCheck.isFile()) || (partFile.exists() && partFile.isFile()) || this->createdFilepaths.contains(destinationPath))
    {
        // Prepare Message Box
        QMessageBox mb;
        QCheckBox *cb = new QCheckBox("Do you want to apply this behaviour to all future incidents "\
                                      "occuring in the current download session?");
        bool applyBehaviour = false;

        mb.setText("The File "+destinationPath+" already exists. How to you want to proceed?");
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
            mb.exec();
            this->isDialogOpen = false;
            QTimer::singleShot(0, this, SLOT(resumeDownloadsAfterDialog())); // We want to resume downloads from the main event loop.
        }
        cb->deleteLater(); // I am not sure if we need to delete the checkbox or QT already takes care of that

        if(mb.clickedButton() == renameButton || this->overwriteBehaviour == OverwriteBehaviour::RENAME) // RENAME
        {
            if(applyBehaviour)
            {
                this->overwriteBehaviour = OverwriteBehaviour::RENAME;
            }
            QFileInfo newFile = this->makeFilepathUnique(destinationPath); // Get a unique filepath. This will just append [0-9]+ until a free name is found.
            destinationPath = newFile.path() + QDir::separator() + newFile.baseName() + "." + newFile.completeSuffix();
            link->setData(Link::DATA_TITLE, newFile.baseName());
            emit downloadVideoRenamed(link, newFile.baseName()); // Also inform the model about the new name.
        }
        if(mb.clickedButton() == skipButton || this->overwriteBehaviour == OverwriteBehaviour::SKIP) // SKIP
        {
            if(applyBehaviour)
            {
                this->overwriteBehaviour = OverwriteBehaviour::SKIP;
            }
            emit downloadVideoSkipped(link);
            this->runningDownloads.remove(link);
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

    }
    this->createdFilepaths.append(destinationPath);
    downloaderProcess->start("youtube-dl --no-mtime "+skipOption+"-f best -o \""+destinationPath+"\" " + link->getData(Link::DATA_LINK).toString() + "");
    emit downloadVideoStarted(link);
}

void YoutubeInterface::resumeDownloadsAfterDialog()
{
    QList<Link*>::iterator it = this->downloadsToResume.begin();
    while(it != this->downloadsToResume.end())
    {
        Link *link = *it;
        it = this->downloadsToResume.erase(it);
        this->downloadVideo(link, this->runningDownloads[link].containerTitle);
    }
}

QFileInfo YoutubeInterface::makeFilepathUnique(QString filepath)
{
    QFileInfo originalFile(filepath);
    QFileInfo newFile(filepath);
    QString newFilepath = filepath;

    // TODO: ERROR HANDLING?
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
    Q_UNUSED(exitCode);

    Process *downloadProcess = (Process*) this->sender();
    Link *link = downloadProcess->getLink();
    this->runningDownloads.remove(link);
    downloadProcess->deleteLater();
    emit downloadVideoFinished(link);
}

void YoutubeInterface::videoDownloadingProcessErrorOccured(Process::ProcessError error)
{
    Process *downloadProcess = (Process*) this->sender();
    this->runningDownloads.remove(downloadProcess->getLink());
    downloadProcess->deleteLater();
    emit downloadVideoFailed(downloadProcess->getLink(), "Could not download video. Error code: " + QString::number(error));
    emit downloadVideoFinished(downloadProcess->getLink());
}

void YoutubeInterface::videoDownloadingProcessStdOut()
{
    Process *downloadProcess = (Process*) this->sender();
    Link *link = downloadProcess->getLink();
    QString content = downloadProcess->readAllStandardOutput();
    this->videoDownloadingProcessHandleStdOut(link, content);

}

void YoutubeInterface::videoDownloadingProcessHandleStdOut(Link *link, QString output)
{
    QRegExp rx("(\\r|\\n)");
    QStringList lines = output.split(rx);
    for(int i = 0; i < lines.size(); i++)
    {
        QString content = lines[i];
        QRegularExpression progressInfo("\\[download\\]\\s+([0-9]+\\.[0-9]+%)\\s+of\\s+([0-9]+\\.[0-9]+[a-zA-Z]+)\\s+at\\s+([0-9]+\\.[0-9]+[a-zA-Z]+\\/s)\\s+ETA\\s+([0-9]+:[0-9]+)");
        QRegularExpressionMatch m = progressInfo.match(content);
        if (m.hasMatch()) {
            QString percentage = m.captured(1);
            QString maxsize = m.captured(2);
            QString speed = m.captured(3);
            QString remaining = m.captured(3);
            emit downloadVideoUpdateProgress(link, percentage, maxsize, speed, remaining);
            continue;
        }
        //\[download\] 100% of ([0-9]+\.[0-9]+MiB)(?:\sin\s([0-9]+:[0-9]+))?

        QRegularExpression finishinfo("\\[download\\]\\s100%\\sof\\s([0-9]+\\.[0-9]+[a-zA-Z]+)(?:\\sin\\s([0-9]+:[0-9]+))?");
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
            continue;
        }
    }
}

void YoutubeInterface::videoDownloadingProcessStdErr()
{
    Process *downloadProcess = (Process*) this->sender();
    QString a = downloadProcess->readAllStandardError();
    emit downloadVideoFailed(downloadProcess->getLink(), a);
    QFile videoFilePart(this->runningDownloads[downloadProcess->getLink()].fullpath+".part");
    if(videoFilePart.exists())
    {
        videoFilePart.remove();
    }
    downloadProcess->kill();
}

// Process Video Extraction Slots
void YoutubeInterface::videoExtractionProcessEnd(int exitCode)
{
    Q_UNUSED(exitCode);

    Process *extractorProcess = (Process*) this->sender();
    extractorProcess->deleteLater();

    QByteArray stdout = extractorProcess->readAllStandardOutput();
    QByteArray stderr = extractorProcess->readAllStandardError();
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
        emit extractedVideoInformationFailed("Corrupted json returned");
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
