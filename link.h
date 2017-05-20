#ifndef LINK_H
#define LINK_H

#include <QString>
#include <QVariant>

class Link
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
        DATA_IS_FAILED
    };

    Link(QString title, QString link, QString progress);
    Link(const Link &other) = default;

    QVariant getData(Link::Data data);
    void setData(Link::Data data, QVariant value);

    static const int DISPLAY_MAX_PROPERTIES = 2;

private:
    QString mTitle;
    QString mLink;
    QString mProgress;
    bool mIsStarted;
    bool mIsFinished;
    bool mIsFailed;
    bool mIsContainer;
};

#endif // LINK_H
