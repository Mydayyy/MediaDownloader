#include "settings.h"

Settings::Settings(bool inMemory)
{
    this->isInMemory = inMemory;
    if(this->isInMemory) {
        return;
    }
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
    if(isInMemory) {
        return this->inMemorySettings.value(key);
    } else {
        return this->settings->value(key);
    }
}

QVariant Settings::get(QString key, QVariant defVal)
{
    if(isInMemory) {
        return this->inMemorySettings.value(key, defVal);
    } else {
        return this->settings->value(key, defVal);
    }

}

void Settings::set(QString key, QVariant val)
{
    if(isInMemory) {
        this->inMemorySettings.insert(key, val);
    } else {
        this->settings->setValue(key, val);
        this->settings->sync();
    }
}
