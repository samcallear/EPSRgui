#ifndef MAKEMOLLATTDIALOG_H
#define MAKEMOLLATTDIALOG_H

#include <QDialog>
#include "ui_makemollattdialog.h"

class MainWindow;

class MakeMolLattDialog : public QDialog
{
    Q_OBJECT

private:
    MainWindow *mainWindow_;

public:
    MakeMolLattDialog(MainWindow *parent = 0);
    Ui::makeMolLattDialog ui;
    int aCells();
    int bCells();
    int cCells();

private slots:
    void on_okButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);
    void check();
};

#endif
