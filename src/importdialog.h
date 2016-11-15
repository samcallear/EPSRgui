#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include <QDir>
#include "ui_importdialog.h"

class MainWindow;

class ImportDialog : public QDialog
{
    Q_OBJECT

private:
    QString workingDir_;
    QString newWorkingDir_;
    MainWindow *mainWindow_;
    QString epsrDir_;
    QString projectName_;
    QString atoFileName_;
    QString epsrInpFileName_;
    QString scriptFile_;

private slots:
    void on_importButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);
    void import();

public:
    ImportDialog(MainWindow *parent = 0);
    Ui::importDialog ui;
    QString getWorkingDir();
    QString getProjectName();
    QString getScriptFile();
//    QString getAtoFileName();
//    QString getEpsrInpName();

public slots:
    void on_browseFolderButton_clicked(bool checked);
    void on_browseBoxButton_clicked(bool checked);
    void on_browseInpButton_clicked(bool checked);
    void on_browseScriptButton_clicked(bool checked);
    void on_addComponentButton_clicked(bool checked);
    void on_removeComponentButton_clicked(bool checked);
    void on_browseNewFolderButton_clicked(bool checked);

};

#endif
