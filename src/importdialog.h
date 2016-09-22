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
    MainWindow *mainWindow_;

private slots:
    void on_importButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);
    void import();

public:
    ImportDialog(MainWindow *parent = 0);
    Ui::importDialog ui;
    QString getProjectName();
    QString getAtoFileName();
    QString getEpsrInpName();

public slots:
    void on_projectNameBrowseButton_clicked(bool checked);
    void on_boxAtoBrowseButton_clicked(bool checked);
    void on_epsrInpBrowseButton_clicked(bool checked);
};

#endif
