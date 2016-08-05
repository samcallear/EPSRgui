#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QRadioButton>
#include "ui_plotdialog.h"
#include "array.h"
#include <QDir>

class MainWindow;

class PlotDialog : public QDialog
{
	Q_OBJECT

private:
    QDir currentDir;
    QString workingDir_;
    QString atoFileName_;
    QString baseFileName_;
    MainWindow *mainWindow_;
    QString plotFileExt_;
    int dataColumn_;

private slots:
    void on_plotButton_clicked(bool checked);
    void on_closeButton_clicked(bool checked);
    void on_addPairButton_clicked(bool checked);
    void on_removePairButton_clicked(bool checked);
    void plotZoom(QWheelEvent *event);
    void on_standardPlotList_itemClicked(QListWidgetItem *item);
    void on_outputPlotList_itemClicked(QListWidgetItem *item);
    void on_optionsButton_clicked(bool checked);

public:
    PlotDialog(MainWindow *parent = 0);
    Ui::plotDialog ui;
    QString getFileName();

    //prep outputPlotList
    void populateOutputsList();

    // prep for plots involving experimental datasets
    void getplottype();
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
    bool yPlot();
    bool histPlot();
    bool datasetPlot();
    bool Eplot();
    bool Rplot();
    bool Pplot();
    bool Ereqplot();
    bool partialsplot();
    bool gofrintraplot();
    bool sqterplot();
    bool sqtraplot();
    bool ereqPlot();

    //plotting
    QCustomPlot customPlot;

public slots:
    void showPointToolTip(QMouseEvent *event);
	
};

#endif
