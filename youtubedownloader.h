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
#include "constants.h"
#include "tablemodel.h"
#include "youtubeinterface.h"
#include "settingsmanager.h"

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
    void extractedLinkInformation(QList<Link*> videos, QString playlistTitle);
    void extractLinkInformationFailed(QString stderr, bool reportError = false);

    // DOWNLOADING
    void downloadNext();
    void downloadVideoFailed(Link *link, QString error);
    void downloadVideoSkipped(Link *link);
    void downloadVideoRenamed(Link *link, QString newName);
    void downloadVideoUpdateProgress(Link *link, QString percentage, QString maxsize, QString speed, QString remaining);
    void downloadVideoUpdateProgressLast(Link *link, QString maxsize, QString time);
    void downloadVideoFinished(Link *link);
    void downloadWatchdog();
};

#endif // YOUTUBEDOWNLOADER_H
