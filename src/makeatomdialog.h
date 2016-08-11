#ifndef MAKEATOMDIALOG_H
#define MAKEATOMDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QComboBox>
#include <QDir>
#include "ui_makeatomdialog.h"

class MainWindow;

class MakeAtomDialog : public QDialog
{
    Q_OBJECT

private:
    QDir currentDit_;
    QString workingDir_;
    MainWindow *mainWindow_;

private slots:
    void on_okButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);
    void checkFiles();

public:
    MakeAtomDialog(MainWindow *parent = 0);
    Ui::makeAtomDialog ui;
    QString getName();
    QString getEpsilon();
    QString getSigma();
    QString getMass();
    QString getCharge();
    QString getSymbol();
};

#endif

