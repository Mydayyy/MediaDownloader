#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <QString>
#include <QVariant>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QApplication>

const QMap<QString, QVariant> defaultSettings{
    {"concurrentDownloads", 1},
    {"createContainerSubfolder", true},
    {"downloadSavePath", "./downloads"},
    {"extractOnlyAudio", false},
    {"audioFormat", "mp3"}
};

class Settings
{
public:
    static Settings& getInstance()
    {
        static Settings instance(false);
        return instance;
    }

    Settings(bool inMemory = true);
    ~Settings();

private:
    QSettings *settings = nullptr;
    QMap<QString, QVariant> inMemorySettings;

    bool isInMemory;
public:
    Settings(Settings const&) = default;
    void operator=(Settings const&)  = delete;

    void set(QString key, QVariant val);
    QVariant get(QString key);
    QVariant get(QString key, QVariant defVal);
    bool hasKey(QString key);
};

#endif // SETTINGSMANAGER_H
