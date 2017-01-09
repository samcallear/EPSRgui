#ifndef REMOVECOMPONENTDIALOG_H
#define REMOVECOMPONENTDIALOG_H

#include <QDialog>
#include "ui_removecomponentdialog.h"

class MainWindow;

class RemoveComponentDialog : public QDialog
{
    Q_OBJECT

private:
    MainWindow *mainWindow_;
    QStringList atoFileList_;

public:
    RemoveComponentDialog(MainWindow *parent = 0);
    Ui::removeComponentDialog ui;
    int returnComponent();

public slots:
    void on_removeButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);
    void check();
};

#endif
