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
    QFileDialog fileBrowser(this, "Choose a download location", Settings::getInstance().get("downloadSavePath").toString());
    fileBrowser.setFileMode(QFileDialog::DirectoryOnly);
    if(fileBrowser.exec() == QDialog::Accepted)
    {
        if(fileBrowser.selectedFiles().size() > 0)
        {
            QString savePath = fileBrowser.selectedFiles()[0];
            Settings::getInstance().set("downloadSavePath", savePath);
            ui->inputSavePath->setText(savePath);
        }
    }
}

// Also we need to watch whether the user just enters a path by himself and write it to the config
void WindowSettings::on_inputSavePath_editingFinished()
{
    Settings::getInstance().set("downloadSavePath", this->ui->inputSavePath->text());
}

void WindowSettings::on_spinboxConcurrentDownloadsChanged(int newValue)
{
    Settings::getInstance().set("concurrentDownloads", QVariant(newValue));
}

void WindowSettings::on_checkboxCreateContainerSubfolder_toggled(bool checked)
{
    Settings::getInstance().set("createContainerSubfolder", QVariant(checked));
}

void WindowSettings::on_checkboxOnlyAudio_toggled(bool checked)
{
    Settings::getInstance().set("extractOnlyAudio", QVariant(checked));
}

void WindowSettings::on_comboAudioFormat_currentTextChanged(const QString &format)
{
    Settings::getInstance().set("audioFormat", QVariant(format));
}

void WindowSettings::on_buttonCloseSettings_clicked()
{
    this->close();
}

void WindowSettings::initializeCurrentSettings()
{
    ui->inputSavePath->setText(Settings::getInstance().get("downloadSavePath").toString());
    ui->inputConcurrentDownloads->setValue(Settings::getInstance().get("concurrentDownloads").toInt());
    ui->checkboxCreateContainerSubfolder->setChecked(Settings::getInstance().get("createContainerSubfolder").toBool());
    ui->checkboxOnlyAudio->setChecked(Settings::getInstance().get("extractOnlyAudio").toBool());
    ui->comboAudioFormat->setItemText(0, Settings::getInstance().get("audioFormat").toString());
}

































