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
#include "tablemodel.h"
#include "youtubeinterface.h"
#include "settings.h"

class MediaDownloader : public QObject
{
    Q_OBJECT
public:
    explicit MediaDownloader(TableModel *tableModel, QObject *parent = nullptr);
    ~MediaDownloader();

    void addLink(QString url);
    void startDownload();
    void stopDownload();
    void extractLinkInformation(QString url, bool reportErrors);

    void clipboardChanged(QString newText);

    TableModel *tableModel;
    YoutubeInterface *youtubeInterface;

    int pendingExtractionProcesses = 0;
    int pendingDownloadProcesses = 0;
    QTimer timerDownloadWatchdog;

public slots:
    void downloadWatchdog();

    // INFORMATION EXTRACTION
    void extractedLinkInformation(QList<MediaObject*> videos, QString playlistTitle);
    void extractLinkInformationFailed(QString sterr, bool reportError = false);
    void downloadPostponed();

    // DOWNLOADING
    void downloadNext();
    void downloadVideoFailed(MediaObject *link, QString error);
    void downloadVideoSkipped(MediaObject *link);
    void downloadVideoRenamed(MediaObject *link, QString newName);
    void downloadVideoUpdateProgress(MediaObject *link, QString percentage, QString maxsize, QString speed, QString remaining);
    void downloadVideoDownloadFinished(MediaObject *link, QString maxsize, QString time);
    void downloadVideoFinished(MediaObject *link);
    void endDownload(MediaObject *link);
};

#endif // YOUTUBEDOWNLOADER_H
