#include "mediaobject.h"

MediaObject::MediaObject(QString title, QString link, QString progress = "") :
    mTitle(title),
    mLink(link),
    mProgress(progress),
    mIsStarted(false),
    mIsFinished(false),
    mIsFailed(false),
    mIsSkipped(false),
    mIsContainer(false),
    mMaxSize(""),
    mSpeed(""),
    mTime(""),
    mDownloadedSize(""),
    treeNode(nullptr)
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
    case MediaObject::DATA_IS_SKIPPED: return mIsSkipped;
    case MediaObject::DATA_MAX_SIZE: return mMaxSize;
    case MediaObject::DATA_SPEED: return mSpeed;
    case MediaObject::DATA_TIME: return mTime;
    case MediaObject::DATA_DOWNLOADED_SIZE: return mDownloadedSize;
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
    case MediaObject::DATA_IS_SKIPPED: mIsSkipped = value.toBool(); return;
    case MediaObject::DATA_MAX_SIZE: mMaxSize = value.toString(); return;
    case MediaObject::DATA_SPEED: mSpeed = value.toString(); return;
    case MediaObject::DATA_TIME: mTime = value.toString(); return;
    case MediaObject::DATA_DOWNLOADED_SIZE: mDownloadedSize = value.toString(); return;
    default: throw; // TODO: proper error handling
    }
}

Settings *MediaObject::getSettings()
{
    return &this->settings;
}

QVariant MediaObject::getSettingsValue(QString key)
{
    if(this->treeNode) {
        return this->treeNode->getSettingsValue(key);
    }
    return Settings::getInstance().get(key);
}

void MediaObject::setTreeNode(TreeNode *treeNode)
{
    this->treeNode = treeNode;
}

TreeNode *MediaObject::getTreeNode()
{
    return this->treeNode;
}
