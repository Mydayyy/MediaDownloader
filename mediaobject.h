#ifndef LINK_H
#define LINK_H

#include <QString>
#include <QVariant>
#include "settings.h"

class MediaObject
{
public:
    enum Data {
        DATA_INVALID,
        DATA_LINK,
        DATA_TITLE,
        DATA_PROGRESS,
        DATA_IS_STARTED,
        DATA_IS_FINISHED,
        DATA_IS_CONTAINER,
        DATA_IS_FAILED,
        DATA_IS_SKIPPED,
        DATA_DOWNLOADED_SIZE, // Indiciates the already downloaded amount
        DATA_SPEED, // Indicates the current speed
        DATA_MAX_SIZE, // Indicates the size of the download
        DATA_TIME // Indicates the estimate time until the download finishes or the time needed when the download has finished
    };

    MediaObject(QString title, QString link, QString progress);
    MediaObject(const MediaObject &other) = default;

    QVariant getData(MediaObject::Data data);
    void setData(MediaObject::Data data, QVariant value);

    Settings *getSettings();

    static const int DISPLAY_MAX_PROPERTIES = 2;

private:
    QString mTitle;
    QString mLink;
    QString mProgress;
    bool mIsStarted;
    bool mIsFinished;
    bool mIsFailed;
    bool mIsSkipped;
    bool mIsContainer;
    QString mMaxSize;
    QString mSpeed;
    QString mTime;
    QString mDownloadedSize;

    Settings settings;
};

#endif // LINK_H
