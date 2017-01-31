#ifndef SETTINGSDIALOG
#define SETTINGSDIALOG

#include <QDialog>
#include <QWidget>
#include "ui_settingsdialog.h"
#include <QDir>
#include <QSettings>

class MainWindow;

class SettingsDialog : public QDialog
{
    Q_OBJECT

private:
    QDir currentDir;
    MainWindow *mainWindow_;
    void readSettings();

private slots:
    void on_okButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);
    void writeSettingsFile();

public:
    SettingsDialog(MainWindow *parent);
    Ui::settingsDialog ui;

public slots:
    void on_browseEPSRDirButton_clicked(bool checked);
    void on_browseEPSRBinDirButton_clicked(bool checked);
    void on_browseVisualiserExeButton_clicked(bool checked);

};

#endif
