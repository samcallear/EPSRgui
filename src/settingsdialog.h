#ifndef SETTINGSDIALOG
#define SETTINGSDIALOG

#include <QtWidgets\QDialog>
#include <QtWidgets\QWidget>
#include "ui_settingsdialog.h"
#include <QtCore\QDir>

class MainWindow;

class SettingsDialog : public QDialog
{
    Q_OBJECT

private:
    QDir currentDir;
    MainWindow *mainWindow_;
    void readSettings();
    void writeSettingsFile();

private slots:
    void on_okButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);

public:
    SettingsDialog(MainWindow *parent);
    Ui::settingsDialog ui;
    QString getPrefEPSRdir();
    QString getPrefVisExe();

public slots:
    void on_browseEPSRDirButton_clicked(bool checked);
    void on_browseVisualiserExeButton_clicked(bool checked);

};

#endif
