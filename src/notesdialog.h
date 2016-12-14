#ifndef NOTESDIALOG_H
#define NOTESDIALOG_H

#include <QDialog>
#include <QWidget>
#include "ui_notesdialog.h"

class MainWindow;

class NotesDialog : public QDialog
{
    Q_OBJECT

private:
    MainWindow *mainWindow_;
    QString workingDir_;

    void readNotes();

private slots:
    void on_saveButton_clicked(bool checked);
    void on_saveCloseButton_clicked(bool checked);
    void on_cancelButton_clicked(bool checked);
    bool save();
    void closeDialog();

public:
    NotesDialog(MainWindow *parent = 0);
    Ui::notesDialog ui;
};

#endif
