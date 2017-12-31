#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <QString>
#include <QVariant>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QApplication>


class Settings
{
public:
    static Settings& getInstance()
    {
        static Settings instance;
        return instance;
    }

    Settings();
    ~Settings();

private:
    QSettings *settings;

public:
    Settings(Settings const&) = delete;
    void operator=(Settings const&)  = delete;

    void set(QString key, QVariant val);
    QVariant get(QString key);
    QVariant get(QString key, QVariant defVal);
};

#endif // SETTINGSMANAGER_H
