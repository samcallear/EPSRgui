#ifndef ADDATODIALOG_H
#define ADDATODIALOG_H

#include "ui_addatodialog.h"
#include <QDialog>

class MainWindow;

class AddAtoDialog : public QDialog
{
    Q_OBJECT

private:
    MainWindow *mainWindow_;

private slots:
    void on_okButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);
    void checkEntries();
    void on_addAtoButton_clicked(bool checked);
    void on_removeAtoButton_clicked(bool checked);

public:
    AddAtoDialog(MainWindow *parent = 0);
    Ui::addAtoDialog ui;
    QString getContainer();
    QStringList getAtoFiles();
    QStringList getNumberMols();

};

#endif // ADDATODIALOG_H
