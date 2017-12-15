#include "windowsettings.h"
#include "ui_windowsettings.h"

WindowSettings::WindowSettings(QWidget *parent) :
    QWidget(parent, Qt::Tool),
    ui(new Ui::WindowSettings)
{
    ui->setupUi(this);

    this->initializeCurrentSettings(); // Load current settings and apply them to the controls

    connect(ui->inputConcurrentDownloads, SIGNAL(valueChanged(int)), this, SLOT(on_spinboxConcurrentDownloadsChanged(int)));
}

WindowSettings::~WindowSettings()
{
    delete ui;
}

// Filebrowsedialog which will be opened when the user clicks browse to select his downloadpath
void WindowSettings::on_buttonBrowseFiles_clicked()
{
    QFileDialog fileBrowser(this, "Choose a download location", SettingsManager::getInstance().get("downloadSavePath", DEFAULT_DOWNLOAD_SAVE_PATH).toString());
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

// Also we need to watch whether the user just enters a path by himself and write it to the config
void WindowSettings::on_inputSavePath_editingFinished()
{
    SettingsManager::getInstance().set("downloadSavePath", this->ui->inputSavePath->text());
}

void WindowSettings::on_spinboxConcurrentDownloadsChanged(int newValue)
{
    SettingsManager::getInstance().set("concurrentDownloads", QVariant(newValue));
}

void WindowSettings::on_checkboxCreateContainerSubfolder_toggled(bool checked)
{
    SettingsManager::getInstance().set("createContainerSubfolder", QVariant(checked));
}

void WindowSettings::on_checkboxOnlyAudio_toggled(bool checked)
{
    SettingsManager::getInstance().set("extractOnlyAudio", QVariant(checked));
}

void WindowSettings::on_comboAudioFormat_currentTextChanged(const QString &format)
{
    SettingsManager::getInstance().set("audioFormat", QVariant(format));
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
    ui->checkboxOnlyAudio->setChecked(SettingsManager::getInstance().get("extractOnlyAudio", QVariant(DEFAULT_EXTRACT_AUDIO)).toBool());
    ui->comboAudioFormat->setItemText(0, SettingsManager::getInstance().get("audioFormat", QVariant(DEFAULT_AUDIO_FORMAT)).toString());
}

































