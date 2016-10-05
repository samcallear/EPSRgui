#ifndef PLOTBOXDIALOG_H
#define PLOTBOXDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QDir>
#include "ui_plotboxdialog.h"

class MainWindow;

class PlotBoxDialog : public QDialog
{
    Q_OBJECT

private:
    QString atoFileName_;
    QString workingDir_;
    QStringList atoAtomLabels_;
    QString epsrBinDir_;
    MainWindow *mainWindow_;

private slots:
    void on_plotBoxButton_clicked(bool checked);

public:
    PlotBoxDialog(MainWindow *parent = 0);
    Ui::plotBoxDialog ui;
    QString getEPSRdir();
    QString getEPSRname();
};

#endif
