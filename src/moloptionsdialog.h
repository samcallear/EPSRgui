#ifndef MOLOPTIONSDIALOG_H
#define MOLOPTIONSDIALOG_H

#include <QtWidgets\QDialog>
#include <QtWidgets\QWidget>
#include <QtWidgets\QComboBox>
#include "ui_moloptionsdialog.h"

class MolOptionsDialog : public QDialog
{
    Q_OBJECT

private:

private slots:
    void on_molOkButton_clicked(bool checked);
    void on_molCancelButton_clicked(bool checked);

public:
    MolOptionsDialog(QWidget *parent = 0);
    Ui::molOptionsDialog ui;

public slots:
    int getMolCharge();
    int getMopacOptions();
};

#endif

