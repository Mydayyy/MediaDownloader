#ifndef WINDOWSETTINGS_H
#define WINDOWSETTINGS_H

#include <QWidget>
#include <QWindow>
#include <QFileDialog>
#include <QDebug>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include "constants.h"
#include "settingsmanager.h"

namespace Ui {
class WindowSettings;
}

class WindowSettings : public QWidget
{
    Q_OBJECT

public:
    explicit WindowSettings(QWidget *parent = 0);
    ~WindowSettings();

private slots:
    void on_buttonBrowseFiles_clicked();
    void on_spinboxConcurrentDownloadsChanged(int newValue);

    void on_buttonCloseSettings_clicked();
    void on_checkboxCreateContainerSubfolder_toggled(bool checked);

private:
    Ui::WindowSettings *ui;

    void initializeCurrentSettings();
};

#endif // WINDOWSETTINGS_H
