#ifndef CREATENEWDIALOG_H
#define CREATENEWDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QRadioButton>
#include "ui_createnewdialog.h"
#include <QDir>

class MainWindow;

class CreateNewDialog : public QDialog
{
    Q_OBJECT

private:
    QDir currentSettingsDir;
    QString projectName_;
    QString workingDir_;
    MainWindow *mainWindow_;

private slots:
    void on_okButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);
    void checkProjectName();

public:
    CreateNewDialog(MainWindow *parent = 0);
    Ui::createNewDialog ui;
    QString getEPSRdir();
    QString getEPSRname();

public slots:
    void on_newBrowseButton_clicked(bool checked);
};

#endif

