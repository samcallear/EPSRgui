#ifndef MESSAGESDIALOG_H
#define MESSAGESDIALOG_H

#include <QDialog>
#include <QWidget>
#include "ui_messagesdialog.h"

class MainWindow;

class MessagesDialog : public QDialog
{
    Q_OBJECT

private:
    MainWindow *mainWindow_;

public slots:
    void refreshMessages();

public:
    MessagesDialog(MainWindow *parent = 0);
    Ui::messagesDialog ui;
};


#endif

