#include "link.h"

Link::Link(QString title, QString link, QString progress = "") :
    mTitle(title),
    mLink(link),
    mProgress(progress),
    mIsStarted(false),
    mIsFinished(false),
    mIsContainer(false),
    mIsFailed(false)
{

}

QVariant Link::getData(Link::Data data)
{
    switch(data) {
    case Link::DATA_LINK: return mLink;;
    case Link::DATA_TITLE: return mTitle;
    case Link::DATA_PROGRESS: return mProgress;
    case Link::DATA_IS_FINISHED: return mIsFinished;
    case Link::DATA_IS_STARTED: return mIsStarted;
    case Link::DATA_IS_CONTAINER: return mIsContainer;
    case Link::DATA_IS_FAILED: return mIsFailed;
    default: throw; // TODO: proper error handling
    }
}

void Link::setData(Link::Data data, QVariant value)
{
    switch(data) {
    case Link::DATA_LINK: mLink = value.toString(); return;
    case Link::DATA_TITLE: mTitle = value.toString(); return;
    case Link::DATA_PROGRESS: mProgress = value.toString(); return;
    case Link::DATA_IS_FINISHED: mIsFinished = value.toBool(); return;
    case Link::DATA_IS_STARTED: mIsStarted = value.toBool(); return;
    case Link::DATA_IS_CONTAINER: mIsContainer = value.toBool(); return;
    case Link::DATA_IS_FAILED: mIsFailed = value.toBool(); return;
    default: throw; // TODO: proper error handling
    }
}
