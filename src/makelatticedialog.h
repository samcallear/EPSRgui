#ifndef MAKELATTICEDIALOG_H
#define MAKELATTICEDIALOG_H


#include <QtWidgets\QDialog>
#include <QtWidgets\QWidget>
#include <QtWidgets\QComboBox>
#include "ui_makelatticedialog.h"

class MakeLatticeDialog : public QDialog
{
    Q_OBJECT

private:

private slots:
    void on_okButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);

public:
    MakeLatticeDialog(QWidget *parent = 0);
    Ui::makeLatticeDialog ui;

};

#endif
