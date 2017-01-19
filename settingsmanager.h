#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <QString>
#include <QVariant>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QApplication>


class SettingsManager
{
public:
    static SettingsManager& getInstance()
    {
        static SettingsManager instance;
        return instance;
    }

private:
    SettingsManager();
    ~SettingsManager();
    QSettings *settings;

public:
    SettingsManager(SettingsManager const&) = delete;
    void operator=(SettingsManager const&)  = delete;

    void set(QString key, QVariant val);
    QVariant get(QString key);
    QVariant get(QString key, QVariant defVal);
};

#endif // SETTINGSMANAGER_H
