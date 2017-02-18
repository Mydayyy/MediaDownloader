#ifndef YOUTUBEINTERFACE_H
#define YOUTUBEINTERFACE_H

#include <QObject>
#include "process.h"
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QCheckBox>
#include <QStringList>
#include <QEventLoop>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "link.h"
#include "settingsmanager.h"
#include "constants.h"

enum OverwriteBehaviour {
    NONE,
    OVERWRITE,
    RENAME,
    SKIP
};

struct DownloadData {
    QString name;
    QString fullpath;
    QString containerTitle;
};

class YoutubeInterface : public QObject
{
    Q_OBJECT
public:
    YoutubeInterface();
    void extractVideoInformation(QString url, bool messageBoxForErrors = true);

    void downloadVideo(Link *link, QString containerTitle = "");
    void resetDownloadSession();

    bool isDialogOpen = false;
private:
    QFileInfo makeFilepathUnique(QString filepath);
    // Used to remember used filepaths and not reuse them,
    // in case another download starts with the same name before the first download created the file
    QStringList createdFilepaths;
    OverwriteBehaviour overwriteBehaviour = OverwriteBehaviour::NONE;

    QMap<Link*, DownloadData> runningDownloads;
    // Used to resume downloads which were aborted due to the dialog being open
    QList<Link*> downloadsToResume;
signals:
    // Extract Video Information
    void extractedVideoInformation(QList<Link*>, QString playlistTitle);
    void extractedVideoInformationFailed(QString stderr, bool reportError);

    // Download Video
    void downloadVideoFailed(Link *link, QString error);
    void downloadVideoSkipped(Link *link);
    void downloadVideoRenamed(Link *link, QString newName);
    void downloadVideoStarted(Link *link);
    void downloadVideoUpdateProgress(Link *link, QString percentage, QString maxsize, QString speed, QString remaining);
    void downloadVideoUpdateProgressLast(Link *link, QString maxsize, QString time);
    void downloadVideoFinished(Link *link);

public slots:
    // QProcess Video Downloading Slots
    void videoDownloadingProcessEnd(int exitCode);
    void videoDownloadingProcessErrorOccured(QProcess::ProcessError error);
    void videoDownloadingProcessStdOut();
    void videoDownloadingProcessHandleStdOut(Link *link, QString output);
    void videoDownloadingProcessStdErr();
    void extractedFilename(int exitcode);
    void resumeDownloadsAfterDialog();

    // QProcess Video Extraction Slots
    void videoExtractionProcessEnd(int exitCode, Process *process, bool reportErrors = true);
    void videoExtractionProcessFinishedReportErrors(int exitCode);
    void videoExtractionProcessFinishedNoErrors(int exitCode);
    void videoExtractionProcessErrorOccured(QProcess::ProcessError error);
};

#endif // YOUTUBEINTERFACE_H
