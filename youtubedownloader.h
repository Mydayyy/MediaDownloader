#ifndef YOUTUBEDOWNLOADER_H
#define YOUTUBEDOWNLOADER_H
#include <QObject>
#include <QString>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include "myconstants.h"
#include "tablemodel.h"
#include "youtubeinterface.h"
#include "settings.h"

class YoutubeDownloader : public QObject
{
    Q_OBJECT
public:
    explicit YoutubeDownloader(TableModel *tableModel, QObject *parent = 0);
    ~YoutubeDownloader();

    void addLink(QString url);
    void startDownload();
    void extractLinkInformation(QString url, bool reportErrors = true);

private:
    TableModel *tableModel;
    YoutubeInterface *ytd;
    //int pendingProcesses = 0;
    int pendingExtractionProcesses = 0;
    int pendingDownloadProcesses = 0;
    bool isDownloading = false;
    QTimer timerDownloadWatchdog;
    
signals:
    void startedOperating();
    void stoppedOperating();

public slots:
    // CLIPBOARD CHANGED
    void clipboardChanged(QString newText);

    // INFORMATION EXTRACTION
    void extractedLinkInformation(QList<MediaObject*> videos, QString playlistTitle);
    void extractLinkInformationFailed(QString sterr, bool reportError = false);

    // DOWNLOADING
    void downloadNext();
    void downloadVideoFailed(MediaObject *link, QString error);
    void downloadVideoSkipped(MediaObject *link);
    void downloadVideoRenamed(MediaObject *link, QString newName);
    void downloadVideoUpdateProgress(MediaObject *link, QString percentage, QString maxsize, QString speed, QString remaining);
    void downloadVideoDownloadFinished(MediaObject *link, QString maxsize, QString time);
    void downloadVideoFinished(MediaObject *link);
    void downloadWatchdog();
};

#endif // YOUTUBEDOWNLOADER_H
