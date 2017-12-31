#include "settingsmanager.h"

Settings::Settings()
{
    QDir settingsPath;
    settingsPath.setPath(QCoreApplication::applicationDirPath());
    settingsPath.mkdir(".cfg");
    settingsPath.cd(".cfg");
    this->settings = new QSettings(settingsPath.filePath(".config"), QSettings::IniFormat);
}

Settings::~Settings()
{
    delete this->settings;
}

QVariant Settings::get(QString key)
{
    return this->settings->value(key);
}

QVariant Settings::get(QString key, QVariant defVal)
{
    return this->settings->value(key, defVal);
}

void Settings::set(QString key, QVariant val)
{
    this->settings->setValue(key, val);
    this->settings->sync();
}
