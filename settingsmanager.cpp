#include "settingsmanager.h"

SettingsManager::SettingsManager()
{
    QDir settingsPath;
    settingsPath.setPath(QCoreApplication::applicationDirPath());
    settingsPath.mkdir(".cfg");
    settingsPath.cd(".cfg");
    this->settings = new QSettings(settingsPath.filePath(".config"), QSettings::IniFormat);
}

SettingsManager::~SettingsManager()
{
    delete this->settings;
}

QVariant SettingsManager::get(QString key)
{
    return this->settings->value(key);
}

QVariant SettingsManager::get(QString key, QVariant defVal)
{
    return this->settings->value(key, defVal);
}

void SettingsManager::set(QString key, QVariant val)
{
    this->settings->setValue(key, val);
    this->settings->sync();
}
