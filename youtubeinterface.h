#ifndef YOUTUBEINTERFACE_H
#define YOUTUBEINTERFACE_H

#include <QObject>
#include "process.h"
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QFileInfo>
#include <QCheckBox>
#include <QStringList>
#include <QEventLoop>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "link.h"
#include "settingsmanager.h"

enum OverwriteBehaviour {
    NONE,
    OVERWRITE,
    RENAME,
    SKIP
};
class YoutubeInterface : public QObject
{
    Q_OBJECT
public:
    YoutubeInterface();
    void extractVideoInformation(QString url);
    void downloadVideo(Link *link);

    bool isDialogOpen = false;
private:
    QFileInfo makeFilepathUnique(QString filepath);
    QStringList createdFilepaths;
    OverwriteBehaviour overwriteBehaviour = OverwriteBehaviour::NONE;


signals:
    // Extract Video Information
    void extractedVideoInformation(QList<Link*>, QString playlistTitle);
    void extractedVideoInformationFailed(QString stderr);

    // Download Video
    void downloadVideoFailed(Link *link, QString error);
    void downloadVideoSkipped(Link *link);
    void downloadVideoRenamed(Link *link, QString newName);
    void downloadVideoStarted(Link *link);
    void downloadVideoUpdateProgress(Link *link, QString percentage, QString maxsize, QString speed, QString remaining);
    void downloadVideoUpdateProgressLast(Link *link, QString maxsize, QString time);
    void downloadVideoFinished(Link *link);

    void dialogOpened();
    void dialogClosed();

public slots:
    // QProcess Video Downloading Slots
    void videoDownloadingProcessEnd(int exitCode);
    void videoDownloadingProcessErrorOccured(QProcess::ProcessError error);
    void videoDownloadingProcessStdOut();
    void videoDownloadingProcessStdErr();

    // QProcess Video Extraction Slots
    void videoExtractionProcessEnd(int exitCode);
    void videoExtractionProcessErrorOccured(QProcess::ProcessError error);
};

#endif // YOUTUBEINTERFACE_H
