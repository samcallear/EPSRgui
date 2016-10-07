#ifndef SETUPOUTPUTDIALOG_H
#define SETUPOUTPUTDIALOG_H

#include <QDialog>
#include <QWidget>
#include "ui_setupoutputdialog.h"
#include <QDir>

class MainWindow;

class SetupOutputDialog : public QDialog
{
    Q_OBJECT

private:
    QString atoFileName_;
    QString outputFileName_;
    QString outputSetupFileType_;
    QString outputFileExt_;
    QString epsrBinDir_;
    QString workingDir_;
    MainWindow *mainWindow_;

    void makeOutputSetupFile();
    void readOutputSetupFile();
    void updateTables();

private slots:
    void on_saveButton_clicked(bool checked);
    void writeOutputFile();
    void on_cancelButton_clicked(bool checked);
    void on_addTabButton_clicked(bool checked);
    void on_removeTabButton_clicked(bool checked);
    void on_deleteOutputButton_clicked(bool checked);

public:
    SetupOutputDialog(MainWindow *parent = 0);
    Ui::setupOutputDialog ui;

    QStringList outputKeywords;
    QStringList outputValues;
    QStringList outputDescriptions;
};

#endif
