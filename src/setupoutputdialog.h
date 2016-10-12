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
    int outputType_;
    QString coeffFileName_;
    QString epsrBinDir_;
    QString workingDir_;
    int settingEntries_;
    int linesPerEntry_;
    int entry_;
//    QString header_;
    MainWindow *mainWindow_;

    void readOutputSetupFile();
    void updateTables();

private slots:
    void on_calcsList_itemSelectionChanged();
    void on_saveButton_clicked(bool checked);
    void writeOutputFile();
    void on_cancelButton_clicked(bool checked);
    void on_addCalcButton_clicked(bool checked);
    void on_removeCalcButton_clicked(bool checked);
    void on_deleteOutputButton_clicked(bool checked);
    void on_browseButton_clicked(bool checked);

public:
    SetupOutputDialog(MainWindow *parent = 0);
    Ui::setupOutputDialog ui;

    QStringList outputKeywords;
    QStringList outputValues;
    QStringList outputDescriptions;
};

#endif
