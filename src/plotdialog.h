#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <QtWidgets\QDialog>
#include <QtWidgets\QWidget>
#include <QtWidgets\QRadioButton>
#include "ui_plotdialog.h"
#include "array.h"
#include <QtCore\QDir>

class MainWindow;

class PlotDialog : public QDialog
{
	Q_OBJECT

private:
    QDir currentDir;
    QString baseFileName_;
    MainWindow *mainWindow_;

private slots:
    void on_plotButton_clicked(bool checked);
    void on_closeButton_clicked(bool checked);
    void on_addPairButton_clicked(bool checked);
    void on_removePairButton_clicked(bool checked);
    void plotZoom(QWheelEvent *event);

public:
    PlotDialog(MainWindow *parent);
    Ui::plotDialog ui;
    QString getFileName();

    // prep for plots involving experimental datasets
    bool getplottype();
    bool getnDataCol();
    int nDatasets;
    int nDataCol;

    //prep for plots involving number of iterattions
    int iterations;

    //preparing for plots involving partials
    bool readatofile();
    QStringList atomLabels;
    int nPartials;
    QString atomLabel1;
    Array2D<int> ij;
    QList<int> iList;
    QList<int> jList;

    //plot functions
    bool fqplot();
    bool frplot();
    bool Eplot();
    bool Rplot();
    bool Pplot();
    bool Ereqplot();
    bool partialsplot();
    bool gofrintraplot();
    bool sqterplot();
    bool sqtraplot();

    //plotting
    QCustomPlot customPlot;

public slots:
    void showPointToolTip(QMouseEvent *event);
	
};

#endif
