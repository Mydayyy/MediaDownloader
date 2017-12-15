#include "mediaobject.h"

MediaObject::MediaObject(QString title, QString link, QString progress = "") :
    mTitle(title),
    mLink(link),
    mProgress(progress),
    mIsStarted(false),
    mIsFinished(false),
    mIsFailed(false),
    mIsContainer(false)
{

}

QVariant MediaObject::getData(MediaObject::Data data)
{
    switch(data) {
    case MediaObject::DATA_LINK: return mLink;;
    case MediaObject::DATA_TITLE: return mTitle;
    case MediaObject::DATA_PROGRESS: return mProgress;
    case MediaObject::DATA_IS_FINISHED: return mIsFinished;
    case MediaObject::DATA_IS_STARTED: return mIsStarted;
    case MediaObject::DATA_IS_CONTAINER: return mIsContainer;
    case MediaObject::DATA_IS_FAILED: return mIsFailed;
    default: throw; // TODO: proper error handling
    }
}

void MediaObject::setData(MediaObject::Data data, QVariant value)
{
    switch(data) {
    case MediaObject::DATA_LINK: mLink = value.toString(); return;
    case MediaObject::DATA_TITLE: mTitle = value.toString(); return;
    case MediaObject::DATA_PROGRESS: mProgress = value.toString(); return;
    case MediaObject::DATA_IS_FINISHED: mIsFinished = value.toBool(); return;
    case MediaObject::DATA_IS_STARTED: mIsStarted = value.toBool(); return;
    case MediaObject::DATA_IS_CONTAINER: mIsContainer = value.toBool(); return;
    case MediaObject::DATA_IS_FAILED: mIsFailed = value.toBool(); return;
    default: throw; // TODO: proper error handling
    }
}
