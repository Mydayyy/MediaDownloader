#ifndef PROCESS_H
#define PROCESS_H
#include <QProcess>
#include "link.h"


class Process : public QProcess
{
public:
    Process(QObject *parent = Q_NULLPTR);
    ~Process();

    void setLink(Link *link);
    Link *getLink();

private:
    Link *link;
};

#endif // PROCESS_H
