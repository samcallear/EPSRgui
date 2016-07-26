#include "plotdialog.h"
#include "array.h"
#include "qcustomplot.h"
#include "epsrproject.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QString>
#include <QLabel>
#include <QVector>
#include <QGuiApplication>

// Constructor

PlotDialog::PlotDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);
    mainWindow_ = parent;

    workingDir_ = mainWindow_->workingDir();
    atoFileName_ = mainWindow_->atoFileName();
    baseFileName_ = workingDir_+atoFileName_.split(".").at(0);

    ui.listWidget_3->setCurrentRow(0);
    connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(reject()));
    readatofile();
    getnDataCol();

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

    //MouseTracking;
    connect(ui.customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(showPointToolTip(QMouseEvent*)));
    connect(ui.customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(plotZoom(QWheelEvent*)));
}

void PlotDialog::on_plotButton_clicked(bool checked)
{
    ui.customPlot->clearGraphs();
    ui.customPlot->clearItems();
    ui.customPlot->clearPlottables();
    getplottype();
}

void PlotDialog::showPointToolTip(QMouseEvent *event)
{

    double x = this->ui.customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = this->ui.customPlot->yAxis->pixelToCoord(event->pos().y());

//    setToolTip(QString("%1 , %2").arg(x).arg(y));

    QString xstr;
    xstr.sprintf(" %5g", x);
    QString ystr;
    ystr.sprintf(" %5g", y);
    QString value = xstr+","+ystr;
    ui.mousecoord->setText(value);
}

void PlotDialog::plotZoom(QWheelEvent* event)
{
    if (event->modifiers() == Qt::ShiftModifier)
    {
        ui.customPlot->axisRect()->setRangeZoomAxes(ui.customPlot->xAxis,ui.customPlot->xAxis);
    }
    else if (event->modifiers() == Qt::ControlModifier)
    {
        ui.customPlot->axisRect()->setRangeZoomAxes(ui.customPlot->yAxis,ui.customPlot->yAxis);
    }
    else
    {
        ui.customPlot->axisRect()->setRangeZoomAxes(ui.customPlot->xAxis,ui.customPlot->yAxis);
    }
}

void PlotDialog::on_closeButton_clicked(bool checked)
{
    ui.customPlot->clearGraphs();
}

//read ato file and construct atom pair array for plot
bool PlotDialog::readatofile()
{
    QFile file(workingDir_+atoFileName_);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .ato file. Check .ato file exists in folder and is accessible in order to plot g(r)s");
        msgBox.exec();
        return 0;
    }
    QTextStream stream(&file);
    QString line;
    QRegExp atomlabelrx(" ([A-Z][A-Za-z0-9 ]{2}) ([A-Za-z ]{1,2})   ([0-1]{1})");
    if (!atomlabelrx.isValid()) printf("REGEXP is not valid.\n");
    atomLabels.clear();

    do {
        line = stream.readLine();
        if (atomlabelrx.exactMatch(line))
        {
            atomLabels << atomlabelrx.cap(1);
        }

    } while (!line.isNull());

    //close ato file
    file.close();

    //number of entries in atomLabels list
    const int N_atomLabels = atomLabels.count();

    //show atomLabels in dialog
    ui.listWidget->clear();
    ui.listWidget_2->clear();
    ui.listWidget->addItems(atomLabels);
    ui.listWidget->setCurrentRow(0);
    ui.listWidget_2->addItems(atomLabels);
    ui.listWidget_2->setCurrentRow(0);

    // Declare and initialise matrix for column to find atom pair partial in...
    ij.initialise(N_atomLabels,N_atomLabels);

    // Populate it: first atompair_index = 0
    int count = 0;
    for (int i = 0; i < N_atomLabels; ++i)
    {
        for (int j = i; j < N_atomLabels; ++j)
        {
            ij.ref(i,j) = count;
            ij.ref(j,i) = count;
            ++count;
        }
    }
    nPartials = count;

    return 0;
}


bool PlotDialog::getnDataCol()
{
    QString FileName;
    FileName = (baseFileName_+".EPSR.u01");
    QFile file(FileName);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .t01 file. This is required for plotting.");
        msgBox.exec();
        return 0;
    }
    QTextStream stream(&file);
    QString line;
    QStringList legendLine;
    line = stream.readLine();                                 //find dataset names and number of datasets - these are used in all other dataset based plots
    legendLine = line.split(" ", QString::SkipEmptyParts);
    nDatasets = legendLine.size()-2;
    nDataCol = nDatasets*2+1;
    file.close();
    return 0;
}


bool PlotDialog::getplottype()
{
    int ptType = ui.listWidget_3->currentRow();
    if (ptType == 0)
    {
        fqplot();
        return 0;
    }
    else
    if (ptType == 1)
    {
        frplot();
        return 0;
    }
    else
    if (ptType == 2)
    {
        Eplot();
        return 0;
    }
    else
    if (ptType == 3)
    {
        Rplot();
        return 0;
    }
    else
    if (ptType == 4)
    {
        partialsplot();
        return 0;
    }
    else
    if (ptType == 5)
    {
        gofrintraplot();
        return 0;
    }
    else
    if (ptType == 6)
    {
        sqterplot();
        return 0;
    }
    else
    if (ptType == 7)
    {
        sqtraplot();
        return 0;
    }
    else
    return 0;
}

bool PlotDialog::fqplot()
{
    //Filenames and number of datasets
    QString fqmodelFileName;
    fqmodelFileName = (baseFileName_+".EPSR.u01");
    QString fqdataFileName;
    fqdataFileName = (baseFileName_+".EPSR.t01");
    QString fqdiffFileName;
    fqdiffFileName = (baseFileName_+".EPSR.v01");
    QFile fileM(fqmodelFileName);
    QFile fileD(fqdataFileName);
    QFile fileDF(fqdiffFileName);
    int column;

    //open and read data file to array
    if(!fileD.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .t01 file.");
        msgBox.exec();
        return 0;
    }
    QTextStream streamD(&fileD);
    QString lineD;
    QStringList dataLineD;
    QVector<double> xD;
    QVector< QVector<double> > columnsD;
    dataLineD.clear();
    xD.clear();
    columnsD.clear();
    columnsD.resize(nDataCol);
    lineD = streamD.readLine();
    do
    {
        lineD = streamD.readLine();
        dataLineD = lineD.split(" ", QString::SkipEmptyParts);
        if (dataLineD.count() == 0) break;
        xD.append(dataLineD.at(0).toDouble());
        int nColumns = (dataLineD.count() - 1) / 2;
        for (column = 0; column < nColumns; ++column)
        {
            if (ui.logYButton->isChecked() == true)
            {
                columnsD[column].append((dataLineD.at(column*2+1).toDouble())+column+1);
            }
            else
            {
                columnsD[column].append((dataLineD.at(column*2+1).toDouble())+column);
            }
        }
    } while (!lineD.isNull());
    fileD.close();

    //open and read model file to array
    if(!fileM.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .u01 file.");
        msgBox.exec();
        return 0;
    }
    QTextStream streamM(&fileM);
    QString lineM;
    QStringList dataLineM;
    QVector<double> xM;
    QVector< QVector<double> > columnsM;
    dataLineM.clear();
    xM.clear();
    columnsM.clear();
    columnsM.resize(nDataCol);
    lineM = streamM.readLine();
    do
    {
        lineM = streamM.readLine();
        dataLineM = lineM.split(" ", QString::SkipEmptyParts);
        if (dataLineM.count() == 0) break;
        xM.append(dataLineM.at(0).toDouble());
        int nColumns = (dataLineM.count() - 1) / 2;
        for (column = 0; column < nColumns; ++column)
        {
            if (ui.logYButton->isChecked() == true)
            {
                columnsM[column].append((dataLineM.at(column*2+1).toDouble())+column+1);
            }
            else
            {
                columnsM[column].append((dataLineM.at(column*2+1).toDouble())+column);
            }
        }
    } while (!lineM.isNull());
    fileM.close();

    //open and read difference file to array
    if(!fileDF.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .v01 file.");
        msgBox.exec();
        return 0;
    }
    QTextStream streamDF(&fileDF);
    QString lineDF;
    QStringList dataLineDF;
    QVector<double> xDF;
    QVector< QVector<double> > columnsDF;
    dataLineDF.clear();
    xDF.clear();
    columnsDF.clear();
    columnsDF.resize(nDataCol);
    lineDF = streamDF.readLine();
    do
    {
        lineDF = streamDF.readLine();
        dataLineDF = lineDF.split(" ", QString::SkipEmptyParts);
        if (dataLineDF.count() == 0) break;
        xDF.append(dataLineDF.at(0).toDouble());
        int nColumns = (dataLineDF.count() - 1) / 2;
        for (column = 0; column < nColumns; ++column)
        {
            if (ui.logYButton->isChecked() == true)
            {
                columnsDF[column].append((dataLineDF.at(column*2+1).toDouble())+column+1);
            }
            else
            {
                columnsDF[column].append((dataLineDF.at(column*2+1).toDouble())+column);
            }
        }
    } while (!lineDF.isNull());
    fileDF.close();

    // create graph and assign data to it:
    QPen pen;
    QString datafileLabel;
    for (int i=0; i < nDatasets*3; i += 3)
    {
        ui.customPlot->addGraph();
        pen.setColor(QColor(qSin(i*0.3)*100+100, qSin(i*0.6+0.7)*100+100, qSin(i*0.4+0.6)*100+100));
        ui.customPlot->graph(i)->setPen(pen);
        ui.customPlot->graph(i)->setData(xM, columnsM.at(i/3));
        ui.customPlot->addGraph();
        ui.customPlot->graph(i+1)->setPen(pen);
        ui.customPlot->graph(i+1)->setData(xD, columnsD.at(i/3));
        ui.customPlot->graph(i+1)->setLineStyle(QCPGraph::lsNone);
        ui.customPlot->graph(i+1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
        ui.customPlot->addGraph();
        ui.customPlot->graph(i+2)->setData(xDF, columnsDF.at(i/3));
        ui.customPlot->graph(i+2)->setPen(QPen(Qt::gray));
        QCPItemText *dataLabel = new QCPItemText(ui.customPlot);
        ui.customPlot->addItem(dataLabel);
        dataLabel->position->setCoords(20,(i/3)+0.2);
        datafileLabel = mainWindow_->dataFileList.at(i/3);
        dataLabel->setText(qPrintable(datafileLabel));
    }

    if (ui.logXButton->isChecked() == true)
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->xAxis->setScaleLogBase(100);
        ui.customPlot->xAxis->setNumberFormat("eb");
        ui.customPlot->xAxis->setNumberPrecision(0);
        ui.customPlot->xAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->xAxis->setNumberFormat("g");
        ui.customPlot->xAxis->setNumberPrecision(2);
        ui.customPlot->xAxis->setAutoTickStep(true);
    }
    if (ui.logYButton->isChecked() == true)
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->yAxis->setScaleLogBase(100);
        ui.customPlot->yAxis->setNumberFormat("eb");
        ui.customPlot->yAxis->setNumberPrecision(0);
        ui.customPlot->yAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->yAxis->setNumberFormat("g");
        ui.customPlot->yAxis->setNumberPrecision(2);
        ui.customPlot->yAxis->setAutoTickStep(true);
    }

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("Q (Angstrom-1)");
    ui.customPlot->yAxis->setLabel("F(Q)");

    //plot
    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;
}

bool PlotDialog::frplot()
{
    //Filenames and number of datasets
    QString frmodelFileName;
    frmodelFileName = (baseFileName_+".EPSR.x01");
    QString frdataFileName;
    frdataFileName = (baseFileName_+".EPSR.w01");
    QFile fileM(frmodelFileName);
    QFile fileD(frdataFileName);
    int column;

    //open and read data file to array
    if(!fileD.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .w01 file.");
        msgBox.exec();
        return 0;
    }
    QTextStream streamD(&fileD);
    QString lineD;
    QStringList dataLineD;
    QVector<double> xD;
    QVector< QVector<double> > columnsD;
    dataLineD.clear();
    xD.clear();
    columnsD.clear();
    columnsD.resize(nDataCol);
    lineD = streamD.readLine();
    do
    {
        lineD = streamD.readLine();
        dataLineD = lineD.split(" ", QString::SkipEmptyParts);
        if (dataLineD.count() == 0) break;
        xD.append(dataLineD.at(0).toDouble());
        int nColumns = (dataLineD.count() - 1) / 2;
        for (column = 0; column < nColumns; ++column)
        {
            if (ui.logYButton->isChecked() == true)
            {
                columnsD[column].append((dataLineD.at(column*2+1).toDouble())+column+1);
            }
            else
            {
                columnsD[column].append((dataLineD.at(column*2+1).toDouble())+column);
            }
        }
    } while (!lineD.isNull());
    fileD.close();

    //open and read model file to array
    if(!fileM.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .x01 file.");
        msgBox.exec();
        return 0;
    }
    QTextStream streamM(&fileM);
    QString lineM;
    QStringList dataLineM;
    QVector<double> xM;
    QVector< QVector<double> > columnsM;
    dataLineM.clear();
    xM.clear();
    columnsM.clear();
    columnsM.resize(nDataCol);
    lineM = streamM.readLine();
    do
    {
        lineM = streamM.readLine();
        dataLineM = lineM.split(" ", QString::SkipEmptyParts);
        if (dataLineM.count() == 0) break;
        xM.append(dataLineM.at(0).toDouble());
        int nColumns = (dataLineM.count() - 1) / 2;
        for (column = 0; column < nColumns; ++column)
        {
            if (ui.logYButton->isChecked() == true)
            {
                columnsM[column].append((dataLineM.at(column*2+1).toDouble())+column+1);
            }
            else
            {
                columnsM[column].append((dataLineM.at(column*2+1).toDouble())+column);
            }
        }
    } while (!lineM.isNull());
    fileM.close();

    // create graph and assign data to it:
    QPen pen;
    QString datafileLabel;
    for (int i=0; i < nDatasets*2; i += 2)
    {
        ui.customPlot->addGraph();
        pen.setColor(QColor(qSin(i*0.3)*100+100, qSin(i*0.6+0.7)*100+100, qSin(i*0.4+0.6)*100+100));
        ui.customPlot->graph(i)->setPen(pen);
        ui.customPlot->graph(i)->setData(xM, columnsM.at(i/2));
        ui.customPlot->addGraph();
        ui.customPlot->graph(i+1)->setPen(pen);
        ui.customPlot->graph(i+1)->setData(xD, columnsD.at(i/2));
        ui.customPlot->graph(i+1)->setLineStyle(QCPGraph::lsNone);
        ui.customPlot->graph(i+1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
        QCPItemText *dataLabel = new QCPItemText(ui.customPlot);
        ui.customPlot->addItem(dataLabel);
        dataLabel->position->setCoords(16,(i/2)+0.2);
        datafileLabel = mainWindow_->dataFileList.at(i/2);
        dataLabel->setText(qPrintable(datafileLabel));
    }

    if (ui.logXButton->isChecked() == true)
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->xAxis->setScaleLogBase(100);
        ui.customPlot->xAxis->setNumberFormat("eb");
        ui.customPlot->xAxis->setNumberPrecision(0);
        ui.customPlot->xAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->xAxis->setNumberFormat("g");
        ui.customPlot->xAxis->setNumberPrecision(2);
        ui.customPlot->xAxis->setAutoTickStep(true);
    }
    if (ui.logYButton->isChecked() == true)
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->yAxis->setScaleLogBase(100);
        ui.customPlot->yAxis->setNumberFormat("eb");
        ui.customPlot->yAxis->setNumberPrecision(0);
        ui.customPlot->yAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->yAxis->setNumberFormat("g");
        ui.customPlot->yAxis->setNumberPrecision(2);
        ui.customPlot->yAxis->setAutoTickStep(true);
    }

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("r (Angstrom)");
    ui.customPlot->yAxis->setLabel("G(r)");

    //plot
    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;
}

bool PlotDialog::Eplot()
{
    QString FileName;
    FileName = (baseFileName_+".EPSR.erg");
    QFile file(FileName);

    //open and read data file to array
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .erg file.");
        msgBox.exec();
        return 0;
    }
    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    QVector<double> x;          //number of iterations
    QVector<double> y;          //energy
    dataLine.clear();
    x.clear();
    y.clear();
    for (iterations = 1; iterations < 1000000; ++iterations)
    {
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        if (dataLine.count() == 0) break;
        y.append(dataLine.at(0).toDouble());
        x.append(iterations);
    }
    file.close();

    // create graph and assign data to it:
    ui.customPlot->addGraph();
    ui.customPlot->graph()->setData(x, y);
    if (ui.logXButton->isChecked() == true)
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->xAxis->setScaleLogBase(100);
        ui.customPlot->xAxis->setNumberFormat("eb");
        ui.customPlot->xAxis->setNumberPrecision(0);
        ui.customPlot->xAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->xAxis->setNumberFormat("g");
        ui.customPlot->xAxis->setNumberPrecision(2);
        ui.customPlot->xAxis->setAutoTickStep(true);
    }
    if (ui.logYButton->isChecked() == true)
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->yAxis->setScaleLogBase(100);
        ui.customPlot->yAxis->setNumberFormat("eb");
        ui.customPlot->yAxis->setNumberPrecision(0);
        ui.customPlot->yAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->yAxis->setNumberFormat("g");
        ui.customPlot->yAxis->setNumberPrecision(3);
        ui.customPlot->yAxis->setAutoTickStep(true);
    }

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("iteration");
    ui.customPlot->yAxis->setLabel("Energy");

    //plot
    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;
}

bool PlotDialog::Rplot()
{
    QString FileName;
    FileName = (baseFileName_+".EPSR.erg");
    QFile file(FileName);

    //open and read data file to array
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .erg file.");
        msgBox.exec();
        return 0;
    }
    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    QVector<double> x;          //number of iterations
    QVector<double> y;          //R-factor
    dataLine.clear();
    x.clear();
    y.clear();
    for (iterations = 1; iterations < 1000000; ++iterations)
    {
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        if (dataLine.count() == 0) break;
        y.append(dataLine.at(2).toDouble());
        x.append(iterations);
    }
    file.close();

    // create graph and assign data to it:
    ui.customPlot->addGraph();
    ui.customPlot->graph()->setData(x, y);
    if (ui.logXButton->isChecked() == true)
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->xAxis->setScaleLogBase(100);
        ui.customPlot->xAxis->setNumberFormat("eb");
        ui.customPlot->xAxis->setNumberPrecision(0);
        ui.customPlot->xAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->xAxis->setNumberFormat("g");
        ui.customPlot->xAxis->setNumberPrecision(2);
        ui.customPlot->xAxis->setAutoTickStep(true);
    }
    if (ui.logYButton->isChecked() == true)
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->yAxis->setScaleLogBase(100);
        ui.customPlot->yAxis->setNumberFormat("eb");
        ui.customPlot->yAxis->setNumberPrecision(0);
        ui.customPlot->yAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->yAxis->setNumberFormat("g");
        ui.customPlot->yAxis->setNumberPrecision(2);
        ui.customPlot->yAxis->setAutoTickStep(true);
    }

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("iteration");
    ui.customPlot->yAxis->setLabel("R-factor");

    //plot
    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;
}

bool PlotDialog::Pplot()
{
    QString FileName;
    FileName = (baseFileName_+".EPSR.erg");
    QFile file(FileName);

    //open and read data file to array
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .erg file.");
        msgBox.exec();
        return 0;
    }
    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    QVector<double> x;          //number of iterations
    QVector<double> y;          //pressure
    dataLine.clear();
    x.clear();
    y.clear();
    for (int iterations = 1; iterations < 1000000; ++iterations)
    {
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        if (dataLine.count() == 0) break;
        y.append(dataLine.at(1).toDouble());
        x.append(iterations);
    }
    file.close();

    // create graph and assign data to it:
    ui.customPlot->addGraph();
    ui.customPlot->graph()->setData(x, y);
    if (ui.logXButton->isChecked() == true)
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->xAxis->setScaleLogBase(100);
        ui.customPlot->xAxis->setNumberFormat("eb");
        ui.customPlot->xAxis->setNumberPrecision(0);
        ui.customPlot->xAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->xAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->xAxis->setNumberFormat("g");
        ui.customPlot->xAxis->setNumberPrecision(2);
        ui.customPlot->xAxis->setAutoTickStep(true);
    }
    if (ui.logYButton->isChecked() == true)
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
        ui.customPlot->yAxis->setScaleLogBase(100);
        ui.customPlot->yAxis->setNumberFormat("eb");
        ui.customPlot->yAxis->setNumberPrecision(0);
        ui.customPlot->yAxis->setSubTickCount(10);
    }
    else
    {
        ui.customPlot->yAxis->setScaleType(QCPAxis::stLinear);
        ui.customPlot->yAxis->setNumberFormat("g");
        ui.customPlot->yAxis->setNumberPrecision(2);
        ui.customPlot->yAxis->setAutoTickStep(true);
    }

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("iteration");
    ui.customPlot->yAxis->setLabel("Pressure");

    //plot
    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;
}

bool PlotDialog::Ereqplot()
{
    QString FileName;
    FileName = (baseFileName_+".EPSR.erg");
    QFile file(FileName);

    //open and read data file to array
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .erg file.");
        msgBox.exec();
        return 0;
    }
    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    QVector<double> x1;          //ereq
    QVector<double> y1;         //fit quality
    QVector<double> x2;          //fitted line x values
    QVector<double> y2;         //fitted line showing gradient of last 50 iterations
    dataLine.clear();
    x1.clear();
    x2.clear();
    y1.clear();
    y2.clear();
    for (int iterations = 1; iterations < 1000000; ++iterations)
    {
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        if (dataLine.count() == 0) break;
        if (dataLine.count() <= 6)
        {
            QMessageBox msgBox;
            msgBox.setText("This plot type is not compatible with this version of EPSR.");
            msgBox.exec();
            file.close();
            break;
        }
        x1.append(dataLine.at(5).toDouble());
        y1.append(dataLine.at(6).toDouble());
        x2.append(dataLine.at(7).toDouble());
        y2.append(dataLine.at(8).toDouble());
    }
    file.close();

    // create graph and assign data to it:
    ui.customPlot->addGraph();
    ui.customPlot->graph()->setData(x1, y1);
    ui.customPlot->addGraph();
    ui.customPlot->graph(1)->setData(x2, y2);
    ui.customPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui.customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("ereq energy");
    ui.customPlot->yAxis->setLabel("quality of fit");

    //plot
    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;
}

bool PlotDialog::partialsplot()
{
    //this is the number of pairs to be plotted
    int numberPairs = iList.count();

    //this is the QVector of QVectors where the data from the pairs listed above will be stored
    QVector< QVector<double> > columns;
    columns.clear();
    columns.resize(numberPairs+1);
    QVector< QVector<double> > columnsCN;
    columnsCN.clear();
    columnsCN.resize(numberPairs+1);
    QVector<double> x;

    //read the data for each pair in the list from the data file by looping over each pair in the list
    for (int pair = 0; pair < numberPairs; ++pair)
    {
        int i = iList.at(pair); // this is the row of the first atom of the pair
        int j = jList.at(pair); // this is the row of the second atom in the pair
        int atompair_index = ij.ref(i,j);
        QString partialsFileName;
        QString CNFileName;

        //the first file contains 100 pairs (even columns from 2 to 200) and atompair_indexes from 0 - 99
        if (atompair_index < 100)
        {
            partialsFileName = (baseFileName_+".EPSR.g01");
            CNFileName = (baseFileName_+".EPSR.z01");
        }
        else
        if (99 < atompair_index && atompair_index < 200)
        {
            partialsFileName = (baseFileName_+".EPSR.g02");
            CNFileName = (baseFileName_+".EPSR.z02");
            atompair_index = atompair_index-100;
        }
        else
        if (199 < atompair_index && atompair_index < 300)
        {
            partialsFileName = (baseFileName_+".EPSR.g03");
            CNFileName = (baseFileName_+".EPSR.z03");
            atompair_index = atompair_index-200;
        }
        else
        if (299 < atompair_index && atompair_index < 400)
        {
            partialsFileName = (baseFileName_+".EPSR.g04");
            CNFileName = (baseFileName_+".EPSR.z04");
            atompair_index = atompair_index-300;
        }
        else
        if (399 < atompair_index && atompair_index < 500)
        {
            partialsFileName = (baseFileName_+".EPSR.g05");
            CNFileName = (baseFileName_+".EPSR.z05");
            atompair_index = atompair_index-400;
        }

        //the first atom pair index = 0 therefore need to x2 and +1 in order to get to correct column.
        int CNpair_column = atompair_index*2+1;
        if (i > j)
        {
            CNpair_column++;
        }

        QFile file(partialsFileName);
        if(!file.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open .g0# file.");
            msgBox.exec();
            return 0;
        }
        QTextStream stream(&file);
        QString line;
        QStringList dataLine;
        dataLine.clear();
        x.clear();
        line = stream.readLine();
        do
        {
            line = stream.readLine();
            dataLine = line.split(" ", QString::SkipEmptyParts);
            if (dataLine.count() == 0) break;
            x.append(dataLine.at(0).toDouble());
            columns[pair].append(dataLine.at(atompair_index*2+1).toDouble()+pair);
        } while (!line.isNull());
        file.close();

        QFile fileCN(CNFileName);
        if(!fileCN.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open .z0# file.");
            msgBox.exec();
            return 0;
        }
        QTextStream streamCN(&fileCN);
        QString lineCN;
        QStringList dataLineCN;
        dataLineCN.clear();
        lineCN = streamCN.readLine();
        do
        {
            lineCN = streamCN.readLine();
            dataLineCN = lineCN.split(" ", QString::SkipEmptyParts);
            if (dataLineCN.count() == 0) break;
            columnsCN[pair].append(dataLineCN.at(CNpair_column).toDouble()+pair);
        } while (!lineCN.isNull());
        fileCN.close();
    }

    // create graph and assign data to it:
    if (ui.CNcheckBox->isChecked())
    {
        QPen pen;
        QString pairLabel;
        for (int i=0; i < numberPairs*2; i += 2)
        {
            ui.customPlot->addGraph();
            pen.setColor(QColor(qSin(i*0.3)*100+100, qSin(i*0.6+0.7)*100+100, qSin(i*0.4+0.6)*100+100));
            ui.customPlot->graph(i)->setPen(pen);
            ui.customPlot->graph(i)->setData(x, columns.at(i/2));
            QCPItemText *dataLabel = new QCPItemText(ui.customPlot);
            ui.customPlot->addItem(dataLabel);
            dataLabel->position->setCoords(12,(i/2)+1.1);
            pairLabel = ui.pairStackPlotList->item(i/2)->text();
            dataLabel->setText(qPrintable(pairLabel));
            ui.customPlot->addGraph();
            ui.customPlot->graph(i+1)->setPen(QPen(Qt::gray));
            ui.customPlot->graph(i+1)->setData(x, columnsCN.at(i/2));
        }
    }
    else
    {
        QPen pen;
        QString pairLabel;
        for (int i=0; i < numberPairs; ++i)
        {
            ui.customPlot->addGraph();
            pen.setColor(QColor(qSin(i*0.3)*100+100, qSin(i*0.6+0.7)*100+100, qSin(i*0.4+0.6)*100+100));
            ui.customPlot->graph(i)->setPen(pen);
            ui.customPlot->graph(i)->setData(x, columns.at(i));
            QCPItemText *dataLabel = new QCPItemText(ui.customPlot);
            ui.customPlot->addItem(dataLabel);
            dataLabel->position->setCoords(12,i+1.1);
            pairLabel = ui.pairStackPlotList->item(i)->text();
            dataLabel->setText(qPrintable(pairLabel));
        }
    }

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("r (Angstrom)");
    ui.customPlot->yAxis->setLabel("g(r)");

    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui.customPlot->xAxis->setRange(0, 15);
    ui.customPlot->yAxis->setRange(0, numberPairs+1);
//    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;
}

bool PlotDialog::gofrintraplot()
{
    //this is the number of pairs to be plotted
    int numberPairs = iList.count();

    //this is the QVector of QVectors where the data from the pairs listed above will be stored
    QVector< QVector<double> > columns;
    columns.clear();
    columns.resize(numberPairs+1);
    QVector<double> x;

    //read the data for each pair in the list from the data file by looping over each pair in the list
    for (int pair = 0; pair < numberPairs; ++pair)
    {
        int i = iList.at(pair); // this is the row of the first atom of the pair
        int j = jList.at(pair); // this is the row of the second atom in the pair
        int atompair_index = ij.ref(i,j);

        QString partialsFileName;

        if (atompair_index < 100)
        {
            partialsFileName = (baseFileName_+".EPSR.y01");
        }
        else
        if (99 < atompair_index && atompair_index < 200)
        {
            partialsFileName = (baseFileName_+".EPSR.y02");
            atompair_index = atompair_index-100;
        }
        else
        if (199 < atompair_index && atompair_index < 300)
        {
            partialsFileName = (baseFileName_+".EPSR.y03");
            atompair_index = atompair_index-200;
        }
        else
        if (299 < atompair_index && atompair_index < 400)
        {
            partialsFileName = (baseFileName_+".EPSR.y04");
            atompair_index = atompair_index-300;
        }
        else
        if (399 < atompair_index && atompair_index < 500)
        {
            partialsFileName = (baseFileName_+".EPSR.y05");
            atompair_index = atompair_index-400;
        }
        else
        if (499 < atompair_index && atompair_index < 600)
        {
            partialsFileName = (baseFileName_+".EPSR.y06");
            atompair_index = atompair_index-500;
        }
        else
        if (599 < atompair_index && atompair_index < 700)
        {
            partialsFileName = (baseFileName_+".EPSR.y07");
            atompair_index = atompair_index-600;
        }
        else
        if (699 < atompair_index && atompair_index < 800)
        {
            partialsFileName = (baseFileName_+".EPSR.y08");
            atompair_index = atompair_index-700;
        }
        else
        if (799 < atompair_index && atompair_index < 900)
        {
            partialsFileName = (baseFileName_+".EPSR.y09");
            atompair_index = atompair_index-800;
        }
        else
        if (899 < atompair_index && atompair_index < 1000)
        {
            partialsFileName = (baseFileName_+".EPSR.y10");
            atompair_index = atompair_index-900;
        }

        QFile file(partialsFileName);
        if(!file.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open .y0# file.");
            msgBox.exec();
            return 0;
        }
        QTextStream stream(&file);
        QString line;
        QStringList dataLine;
        dataLine.clear();
        x.clear();
        line = stream.readLine();
        do
        {
            line = stream.readLine();
            dataLine = line.split(" ", QString::SkipEmptyParts);
            if (dataLine.count() == 0) break;
            x.append(dataLine.at(0).toDouble());
            columns[pair].append(dataLine.at(atompair_index*2+1).toDouble()+pair);
        } while (!line.isNull());
        file.close();
    }

    // create graph and assign data to it:
    QPen pen;
    QString pairLabel;
    for (int i=0; i < numberPairs; ++i)
    {
        ui.customPlot->addGraph();
        pen.setColor(QColor(qSin(i*0.3)*100+100, qSin(i*0.6+0.7)*100+100, qSin(i*0.4+0.6)*100+100));
        ui.customPlot->graph(i)->setPen(pen);
        ui.customPlot->graph(i)->setData(x, columns.at(i));
        QCPItemText *dataLabel = new QCPItemText(ui.customPlot);
        ui.customPlot->addItem(dataLabel);
        dataLabel->position->setCoords(12,i+1.1);
        pairLabel = ui.pairStackPlotList->item(i)->text();
        dataLabel->setText(qPrintable(pairLabel));
    }

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("r (Angstrom)");
    ui.customPlot->yAxis->setLabel("g(r)");

    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui.customPlot->xAxis->setRange(0, 15);
    ui.customPlot->yAxis->setRange(0, numberPairs+1);
//    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;
}

bool PlotDialog::sqterplot()
{
    //this is the number of pairs to be plotted
    int numberPairs = iList.count();

    //this is the QVector of QVectors where the data from the pairs listed above will be stored
    QVector< QVector<double> > columns;
    columns.clear();
    columns.resize(numberPairs+1);
    QVector<double> x;

    //read the data for each pair in the list from the data file by looping over each pair in the list
    for (int pair = 0; pair < numberPairs; ++pair)
    {
        int i = iList.at(pair); // this is the row of the first atom of the pair
        int j = jList.at(pair); // this is the row of the second atom in the pair
        int atompair_index = ij.ref(i,j);

        QString partialsFileName;

        if (atompair_index < 100)
        {
            partialsFileName = (baseFileName_+".EPSR.f01");
        }
        else
        if (99 < atompair_index && atompair_index < 200)
        {
            partialsFileName = (baseFileName_+".EPSR.f02");
            atompair_index = atompair_index-100;
        }
        else
        if (199 < atompair_index && atompair_index < 300)
        {
            partialsFileName = (baseFileName_+".EPSR.f03");
            atompair_index = atompair_index-200;
        }
        else
        if (299 < atompair_index && atompair_index < 400)
        {
            partialsFileName = (baseFileName_+".EPSR.f04");
            atompair_index = atompair_index-300;
        }
        else
        if (399 < atompair_index && atompair_index < 500)
        {
            partialsFileName = (baseFileName_+".EPSR.f05");
            atompair_index = atompair_index-400;
        }

        QFile file(partialsFileName);
        if(!file.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open .f0# file.");
            msgBox.exec();
            return 0;
        }
        QTextStream stream(&file);
        QString line;
        QStringList dataLine;
        dataLine.clear();
        x.clear();
        line = stream.readLine();
        do
        {
            line = stream.readLine();
            dataLine = line.split(" ", QString::SkipEmptyParts);
            if (dataLine.count() == 0) break;
            x.append(dataLine.at(0).toDouble());
            columns[pair].append(dataLine.at(atompair_index*2+1).toDouble()+pair);
        } while (!line.isNull());
        file.close();
    }

    // create graph and assign data to it:
    QPen pen;
    QString pairLabel;
    for (int i=0; i < numberPairs; ++i)
    {
        ui.customPlot->addGraph();
        pen.setColor(QColor(qSin(i*0.3)*100+100, qSin(i*0.6+0.7)*100+100, qSin(i*0.4+0.6)*100+100));
        ui.customPlot->graph(i)->setPen(pen);
        ui.customPlot->graph(i)->setData(x, columns.at(i));
        QCPItemText *dataLabel = new QCPItemText(ui.customPlot);
        ui.customPlot->addItem(dataLabel);
        dataLabel->position->setCoords(18,i+0.1);
        pairLabel = ui.pairStackPlotList->item(i)->text();
        dataLabel->setText(qPrintable(pairLabel));
    }

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("r (Angstrom)");
    ui.customPlot->yAxis->setLabel("g(r)");

    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
//    ui.customPlot->xAxis->setRange(0, 15);
//    ui.customPlot->yAxis->setRange(0, numberPairs+1);
    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;

}

bool PlotDialog::sqtraplot()
{
    //this is the number of pairs to be plotted
    int numberPairs = iList.count();

    //this is the QVector of QVectors where the data from the pairs listed above will be stored
    QVector< QVector<double> > columns;
    columns.clear();
    columns.resize(numberPairs+1);
    QVector<double> x;

    //read the data for each pair in the list from the data file by looping over each pair in the list
    for (int pair = 0; pair < numberPairs; ++pair)
    {
        int i = iList.at(pair); // this is the row of the first atom of the pair
        int j = jList.at(pair); // this is the row of the second atom in the pair
        int atompair_index = ij.ref(i,j);

        QString partialsFileName;

        if (atompair_index < 100)
        {
            partialsFileName = (baseFileName_+".EPSR.s01");
        }
        else
        if (99 < atompair_index && atompair_index < 200)
        {
            partialsFileName = (baseFileName_+".EPSR.s02");
            atompair_index = atompair_index-100;
        }
        else
        if (199 < atompair_index && atompair_index < 300)
        {
            partialsFileName = (baseFileName_+".EPSR.s03");
            atompair_index = atompair_index-200;
        }
        else
        if (299 < atompair_index && atompair_index < 400)
        {
            partialsFileName = (baseFileName_+".EPSR.s04");
            atompair_index = atompair_index-300;
        }
        else
        if (399 < atompair_index && atompair_index < 500)
        {
            partialsFileName = (baseFileName_+".EPSR.s05");
            atompair_index = atompair_index-400;
        }

        QFile file(partialsFileName);
        if(!file.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open .s0# file.");
            msgBox.exec();
            return 0;
        }
        QTextStream stream(&file);
        QString line;
        QStringList dataLine;
        dataLine.clear();
        x.clear();
        line = stream.readLine();
        do
        {
            line = stream.readLine();
            dataLine = line.split(" ", QString::SkipEmptyParts);
            if (dataLine.count() == 0) break;
            x.append(dataLine.at(0).toDouble());
            columns[pair].append(dataLine.at(atompair_index*2+1).toDouble()+pair);
        } while (!line.isNull());
        file.close();
    }

    // create graph and assign data to it:
    QPen pen;
    QString pairLabel;
    for (int i=0; i < numberPairs; ++i)
    {
        ui.customPlot->addGraph();
        pen.setColor(QColor(qSin(i*0.3)*100+100, qSin(i*0.6+0.7)*100+100, qSin(i*0.4+0.6)*100+100));
        ui.customPlot->graph(i)->setPen(pen);
        ui.customPlot->graph(i)->setData(x, columns.at(i));
        QCPItemText *dataLabel = new QCPItemText(ui.customPlot);
        ui.customPlot->addItem(dataLabel);
        dataLabel->position->setCoords(18,(i)+0.1);
        pairLabel = ui.pairStackPlotList->item(i)->text();
        dataLabel->setText(qPrintable(pairLabel));
    }

    // give the axes some labels:
    ui.customPlot->xAxis->setLabel("q (Angstrom-1)");
    ui.customPlot->yAxis->setLabel("s(q)");

    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
//    ui.customPlot->xAxis->setRange(0, 15);
//    ui.customPlot->yAxis->setRange(0, numberPairs+1);
    ui.customPlot->rescaleAxes();
    ui.customPlot->replot();
    return 0;

}

void PlotDialog::on_addPairButton_clicked(bool checked)
{
    QString atom1 = ui.listWidget->currentItem()->text();
    QString atom2 = ui.listWidget_2->currentItem()->text();

    QString atomPair = atom1+"-"+atom2;

    ui.pairStackPlotList->addItem(atomPair);

    iList.append(ui.listWidget->currentRow());
    jList.append(ui.listWidget_2->currentRow());

}

void PlotDialog::on_removePairButton_clicked(bool checked)
{
    int row = ui.pairStackPlotList->currentRow();
    if (ui.pairStackPlotList->count() > 0)
    {
        ui.pairStackPlotList->takeItem(row);
        iList.removeAt(row);
        jList.removeAt(row);
    }
}
