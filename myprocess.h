#ifndef PROCESS_H
#define PROCESS_H
#include <QProcess>
#include "mediaobject.h"


class Process : public QProcess
{
public:
    Process(QObject *parent = Q_NULLPTR);
    ~Process();

    void setLink(MediaObject *link);

    MediaObject *getLink();

    QString getUrl() const;
    void setUrl(const QString &value);

private:
    MediaObject *link;
    QString url;
};

#endif // PROCESS_H
