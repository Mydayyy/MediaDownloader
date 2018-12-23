#include "youtubeinterface.h"

YoutubeInterface::YoutubeInterface()
{

}

void YoutubeInterface::resetDownloadSession()
{
    this->createdFilepaths.clear();
    this->overwriteBehaviour = OverwriteBehaviour::NONE;
}

void YoutubeInterface::extractVideoInformation(QString url, bool messageBoxForErrors)
{
    Process *extractorProcess = new Process(this);
    connect(extractorProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), // on Process Startup Error
            this, SLOT(videoExtractionProcessErrorOccured(QProcess::ProcessError)));
    if(messageBoxForErrors)
    {
        connect(extractorProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(videoExtractionProcessFinishedReportErrors(int, QProcess::ExitStatus))); // on Process Finished
    }
    else
    {
        connect(extractorProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(videoExtractionProcessFinishedNoErrors(int, QProcess::ExitStatus))); // on Process Finished
    }
    qDebug() << "Starting to extract " << url << " reporting errors: " << messageBoxForErrors;
    extractorProcess->setUrl(url);
    extractorProcess->start("youtube-dl --ignore-errors -s -J " + url); // -J  will dump the json, -s will simulate and not actually download anything
}

void YoutubeInterface::videoExtractionProcessFinishedReportErrors(int exitCode, QProcess::ExitStatus exitStatus)
{
    Process *extractorProcess = (Process*) this->sender();
    this->videoExtractionProcessEnd(exitCode, exitStatus, extractorProcess, true);
}

void YoutubeInterface::videoExtractionProcessFinishedNoErrors(int exitCode, QProcess::ExitStatus exitStatus)
{
    Process *extractorProcess = (Process*) this->sender();
    this->videoExtractionProcessEnd(exitCode, exitStatus, extractorProcess, false);
}

void YoutubeInterface::videoExtractionProcessErrorOccured(Process::ProcessError error)
{
    Process *extractorProcess = (Process*) this->sender();
    qDebug() << "Error during extraction for " << extractorProcess->getUrl() << error;
    if(error == QProcess::FailedToStart) {
        extractorProcess->deleteLater();
        emit extractedVideoInformationFailed("Extraction Process could not be started. Error Code: " + QString::number(error), true);
    }
}

// Process Video Extraction Slots
void YoutubeInterface::videoExtractionProcessEnd(int exitCode, QProcess::ExitStatus exitStatus, Process *process, bool reportErrors)
{
    Process *extractorProcess = process;
    extractorProcess->deleteLater();
    if(exitCode != 0) {
        qDebug() << "Error during extraction for " << extractorProcess->getUrl() << exitCode << exitStatus << reportErrors;
        emit extractedVideoInformationFailed("Error extracting video:\n\n" + extractorProcess->readAllStandardError(), reportErrors);
        return;
    }

    QByteArray stout = extractorProcess->readAllStandardOutput();
    if(stout.size() == 0) // Not quite sure whether this can happen.
    {
        qDebug() << "Error during extraction ( stdout empty ) for " << extractorProcess->getUrl() << exitCode << exitStatus << reportErrors;
        emit extractedVideoInformationFailed("No videos found", reportErrors);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(stout);
    if(doc.isNull()) {
        qDebug() << "Error during extraction ( corrupt json ) for " << extractorProcess->getUrl() << exitCode << exitStatus << reportErrors;
        emit extractedVideoInformationFailed("Corrupted json returned", reportErrors);
        return;
    }

    QJsonObject root = doc.object();

    QJsonArray videos;
    QString playlistTitle = "";

    if(root.contains("_type")) {
        QString type = root.value("_type").toString();
        if(type == "playlist") {
            videos = root.value("entries").toArray();
            playlistTitle = root.value("title").toString();
        } else {
            qDebug() << "Error during extraction ( unsupported type ) for " << extractorProcess->getUrl() << exitCode << exitStatus << reportErrors;
            emit extractedVideoInformationFailed("Unsupported type "+type+", please report the error and url where this happened", reportErrors);
            return;
        }
    } else {
        videos.append(root);
    }

    int videoCount = videos.count();
    QList<MediaObject*> videoLinks;

    for(int i = 0; i < videoCount; i++)
    {
        QJsonObject video = videos.at(i).toObject();
        QString title = video["title"].toString();
        QString url = video["webpage_url"].toString();
        MediaObject *link = new MediaObject(title, url, "0%");
        videoLinks.append(link);
    }
    qDebug() << "Successfully extracted " << extractorProcess->getUrl()  << exitCode << exitStatus << reportErrors;
    emit extractedVideoInformation(videoLinks, playlistTitle);
}

void YoutubeInterface::downloadVideo(MediaObject *link, QString containerTitle)
{
    DownloadData data;
    data.fullpath = "";
    data.name = "";
    data.containerTitle = containerTitle;
    this->runningDownloads[link] = data; // Add to running downloads

    QDir path(link->getSettingsValue("downloadSavePath").toString());

    if(!containerTitle.isEmpty()) // In case the subfolder does not exist, create it
    {
        path.mkpath(containerTitle);
        path.cd(containerTitle);
    }

    QFileInfo destinationFile = path.path() + QDir::separator() + "%(title)s.%(ext)s";
    QString destinationPath = destinationFile.path() + QDir::separator() + destinationFile.baseName() + "." + destinationFile.completeSuffix();
    // TODO: POSSIBLE STUCK HERE - ADD ERROR HANDLING
    Process *getFilenameProcess = new Process(this);
    getFilenameProcess->setLink(link); // We need to access the link later in the slots
    connect(getFilenameProcess, SIGNAL(finished(int)), this, SLOT(extractedFilename(int)));
    qDebug() << "Starting to download filename for" << link->getData(MediaObject::DATA_LINK).toString();
    getFilenameProcess->start("youtube-dl --no-mtime --get-filename -o \""+destinationPath+"\" " + link->getData(MediaObject::DATA_LINK).toString() + "");
}

void YoutubeInterface::extractedFilename(int exitCode)
{
    Process *getFilenameProcess = (Process*) this->sender();
    getFilenameProcess->deleteLater();
    MediaObject *link = getFilenameProcess->getLink();
    QString destinationPath = getFilenameProcess->readAllStandardOutput(); // Youtube-dl prints the name of the video in a single line

    if(destinationPath.isEmpty() || exitCode != 0) {
        this->runningDownloads.remove(link);
        qDebug() << "Error during downloading filename ( path empty or error set ) for" << link->getData(MediaObject::DATA_LINK).toString() << exitCode;
        emit downloadVideoFailed(link, getFilenameProcess->readAllStandardError());
        return;
    }

    qDebug() << "Successfully downloaded filename (" << destinationPath << ") for" << link->getData(MediaObject::DATA_LINK).toString() << exitCode;

    Process *downloaderProcess = new Process(this);
    downloaderProcess->setLink(link);
    connect(downloaderProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(videoDownloadingProcessEnd(int, QProcess::ExitStatus)));
    connect(downloaderProcess, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(videoDownloadingProcessErrorOccured(QProcess::ProcessError)));
    connect(downloaderProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(videoDownloadingProcessStdOut()));

    destinationPath = destinationPath.simplified(); // Strip it of \n\r\t and multiple spaces...
    QFileInfo fileCheck(destinationPath);
    QFileInfo partFile(destinationPath+".part"); // TODO: Handle resume download when partfile present
    QFileInfo mkvFile(fileCheck.path() + QDir::separator() + fileCheck.baseName() + ".mkv");
    this->runningDownloads[link].fullpath = destinationPath;

    // When the dialog is open, we abort all downloads until the user made a decision. They will be resumed after.
    qDebug() << "IsDialogOpen:" << isDialogOpen;
    if(this->isDialogOpen) {
        downloaderProcess->deleteLater();
        link->setData(MediaObject::DATA_IS_FAILED, QVariant(false));
        link->setData(MediaObject::DATA_IS_STARTED, QVariant(false));
        link->setData(MediaObject::DATA_IS_SKIPPED, QVariant(false));
        this->runningDownloads.remove(link);
        emit downloadPostponed();
        return;
    }

    QString skipOption = ""; // Used to overwrite the file in case the user made the decision.
    if((fileCheck.exists() && fileCheck.isFile()) || (partFile.exists() && partFile.isFile()) || this->createdFilepaths.contains(destinationPath) || mkvFile.exists())
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
            qDebug() << "Setting isDialogOpen";
            this->isDialogOpen = true;
            mb.exec();
            this->isDialogOpen = false;
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
            link->setData(MediaObject::DATA_TITLE, newFile.baseName());
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

    // Check whether the user only wants to download audio
    QString onlyAudioOption = "";
    if(link->getSettingsValue("extractOnlyAudio").toBool()) {
        onlyAudioOption = "-x --audio-format "+ link->getSettingsValue("audioFormat").toString() +" --audio-quality 0 ";
    }

    this->createdFilepaths.append(destinationPath);
    qDebug() << "youtube-dl --no-part --no-mtime "+skipOption+onlyAudioOption+" -o \""+destinationPath+"\" " + link->getData(MediaObject::DATA_LINK).toString() + "";
    downloaderProcess->start("youtube-dl --no-part --no-mtime "+skipOption+onlyAudioOption+" -o \""+destinationPath+"\" " + link->getData(MediaObject::DATA_LINK).toString() + "");
    qDebug() << "Starting download process for" << link->getData(MediaObject::DATA_LINK).toString();
    emit downloadVideoStarted(link);
}

QFileInfo YoutubeInterface::makeFilepathUnique(QString filepath)
{
    QFileInfo originalFile(filepath);
    QFileInfo newFile(filepath);
    QFileInfo mkvFile(newFile.path() + QDir::separator() + newFile.completeBaseName() + ".mkv");
    QString newFilepath = filepath;

    // TODO: ERROR HANDLING?
    int i = 1;
    while(newFile.exists() || this->createdFilepaths.contains(newFilepath) || mkvFile.exists())
    {
        newFilepath = originalFile.path() + QDir::separator() + originalFile.baseName() + " " + QString::number(i) +"." + originalFile.completeSuffix();
        newFile = QFileInfo(newFilepath);
        mkvFile = newFile.path() + QDir::separator() + newFile.completeBaseName() + ".mkv";
        i++;
    }
    this->createdFilepaths.append(newFilepath);
    return newFile;
}

// SLOTS

// Process Video Downloading Slots
void YoutubeInterface::videoDownloadingProcessEnd(int exitCode, QProcess::ExitStatus exitStatus)
{
    Process *downloadProcess = (Process*) this->sender();
    MediaObject *link = downloadProcess->getLink();

    if(exitStatus == QProcess::CrashExit) {
        // This is handled by videoDownloadingProcessErrorOccured
        qDebug() << "Download process crashed for" << link->getData(MediaObject::DATA_LINK).toString() << exitCode << exitStatus;
        return;
    }

    downloadProcess->deleteLater();

    if(exitCode != 0) {
        qDebug() << "Download process had a nonzero exit code for" << link->getData(MediaObject::DATA_LINK).toString() << exitCode << exitStatus;
        emit downloadVideoFailed(link, "Download Failed ("+QString::number(exitCode)+"):\n\n" + downloadProcess->readAllStandardError());
        this->runningDownloads.remove(link);
        return;
    }

    this->runningDownloads.remove(link);
    qDebug() << "Download sucessfully ended for" << link->getData(MediaObject::DATA_LINK).toString() << exitCode << exitStatus;
    emit downloadVideoFinished(link);
}

void YoutubeInterface::videoDownloadingProcessErrorOccured(Process::ProcessError error)
{
    Process *downloadProcess = (Process*) this->sender();
    downloadProcess->deleteLater();
    MediaObject *link = downloadProcess->getLink();
    qDebug() << "Download process crashed for" << link->getData(MediaObject::DATA_LINK).toString() << error;
    emit downloadVideoFailed(downloadProcess->getLink(), "Download Process Crashed. Error code: " + QString::number(error));
    this->runningDownloads.remove(downloadProcess->getLink());
}

void YoutubeInterface::videoDownloadingProcessStdOut()
{
    Process *downloadProcess = (Process*) this->sender();
    MediaObject *link = downloadProcess->getLink();
    QString content = downloadProcess->readAllStandardOutput();
    qDebug() << "Download process std out for " << link->getData(MediaObject::DATA_LINK).toString() << ":" << content;
    this->videoDownloadingProcessHandleStdOut(link, content);
}

void YoutubeInterface::videoDownloadingProcessHandleStdOut(MediaObject *link, QString output)
{
    QRegExp rx("(\\r|\\n)");
    QStringList lines = output.split(rx);
    for(int i = 0; i < lines.size(); i++) {
        QString content = lines[i];
        if(content.isEmpty()) {
            continue;
        }
        QRegularExpression progressData("\\[download\\]\\s*"
                                        "(?<progress>[0-9]+\\.[0-9]+%)\\s*"
                                        "(?<size_wrapper>of\\s~?(?<size>[0-9]*\\.[0-9]*)(?<unit>[a-zA-Z]+))\\s*"
                                        "(?<speed_wrapper>at\\s+(?:(?<speed>[0-9]+\\.[0-9]+[a-zA-Z\\/]+)|(?:Unknown speed)))\\s*"
                                        "(?<eta_wrapper>ETA\\s(?<eta>[0-9:]+))");
        QRegularExpressionMatch m = progressData.match(content);
        if (m.hasMatch()) {
            QString progress = m.captured("progress");
            QString size = m.captured("size");
            QString unit = m.captured("unit");
            QString speed = m.captured("speed");
            QString eta = m.captured("eta");
            emit downloadVideoUpdateProgress(link, progress, size+unit, speed, eta);
            continue;
        }

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
            emit downloadVideoDownloadFinished(link, maxsize, time);
            continue;
        }
    }
}

