#include "process.h"

Process::Process(QObject *parent) : QProcess(parent)
{

}
Process::~Process()
{

}

void Process::setLink(MediaObject *link)
{
    this->link = link;
}

MediaObject *Process::getLink()
{
    return this->link;
}
