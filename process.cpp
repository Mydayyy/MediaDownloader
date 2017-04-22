#include "process.h"

Process::Process(QObject *parent) : QProcess(parent)
{

}
Process::~Process()
{

}

void Process::setLink(Link *link)
{
    this->link = link;
}

Link *Process::getLink()
{
    return this->link;
}
