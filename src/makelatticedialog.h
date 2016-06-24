#ifndef MAKELATTICEDIALOG_H
#define MAKELATTICEDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QComboBox>
#include <QDir>
#include "ui_makelatticedialog.h"

class MainWindow;

class MakeLatticeDialog : public QDialog
{
    Q_OBJECT

private:
    QDir currentDir_;
    QString workingDir_;
    MainWindow *mainWindow_;
    QString unitFileName_;

    bool readUnitFile();
    bool saveToUnitFile();

private slots:
    void on_okButton_clicked(bool checked);
    void checkEntries();
    void on_cancelButton_clicked(bool checked);
    void on_browseButton_clicked(bool checked);
    void on_addRowButton_clicked(bool checked);
    void on_deleteRowButton_clicked(bool checked);
    void checkAtomLabel(int row, int column);

public:
    MakeLatticeDialog(MainWindow *parent = 0);
    Ui::makeLatticeDialog ui;
    QString cellsAlongA();
    QString cellsAlongB();
    QString cellsAlongC();
    QString unitFileName();
    QStringList atomTypes();
    QString charge();
};

#endif
