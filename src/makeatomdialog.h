#ifndef MAKEATOMDIALOG_H
#define MAKEATOMDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QComboBox>
#include "ui_makeatomdialog.h"

class MakeAtomDialog : public QDialog
{
    Q_OBJECT

private:

private slots:
    void on_okButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);

public:
    MakeAtomDialog(QWidget *parent = 0);
    Ui::makeAtomDialog ui;
    QString getName();
    QString getEpsilon();
    QString getSigma();
    QString getMass();
    QString getCharge();
    QString getSymbol();
    QString getTemp();
    QString getDensity();
};

#endif

