#include "windowsettings.h"
#include "ui_windowsettings.h"

WindowSettings::WindowSettings(QWidget *parent) :
    QWidget(parent, Qt::Tool),
    ui(new Ui::WindowSettings)
{
    ui->setupUi(this);

    this->initializeCurrentSettings();

    connect(ui->inputConcurrentDownloads, SIGNAL(valueChanged(int)), this, SLOT(on_spinboxConcurrentDownloadsChanged(int)));
}

WindowSettings::~WindowSettings()
{
    delete ui;
}

void WindowSettings::on_buttonBrowseFiles_clicked()
{
    QFileDialog fileBrowser(this, "Choose a download location", SettingsManager::getInstance().get("downloadSavePath", QString("")).toString());
    fileBrowser.setFileMode(QFileDialog::DirectoryOnly);
    if(fileBrowser.exec() == QDialog::Accepted)
    {
        if(fileBrowser.selectedFiles().size() > 0)
        {
            QString savePath = fileBrowser.selectedFiles()[0];
            SettingsManager::getInstance().set("downloadSavePath", savePath);
            ui->inputSavePath->setText(savePath);
        }
    }
}


void WindowSettings::on_spinboxConcurrentDownloadsChanged(int newValue)
{
    SettingsManager::getInstance().set("concurrentDownloads", QVariant(newValue));
}

void WindowSettings::on_checkboxCreateContainerSubfolder_toggled(bool checked)
{
    SettingsManager::getInstance().set("createContainerSubfolder", QVariant(checked));
}

void WindowSettings::on_buttonCloseSettings_clicked()
{
    this->close();
}

void WindowSettings::initializeCurrentSettings()
{
    ui->inputSavePath->setText(SettingsManager::getInstance().get("downloadSavePath").toString());
    ui->inputConcurrentDownloads->setValue(SettingsManager::getInstance().get("concurrentDownloads", DEFAULT_CONCURRENT_DOWNLOADS).toInt());
    ui->checkboxCreateContainerSubfolder->setChecked(SettingsManager::getInstance().get("createContainerSubfolder", QVariant(DEFAULT_CREATE_CONTAINER_SUBFOLDER)).toBool());
}






























