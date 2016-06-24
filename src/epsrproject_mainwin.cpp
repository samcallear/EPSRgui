#include "epsrproject.h"
#include "ui_epsrproject.h"
#include "plotdialog.h"
#include "createnewdialog.h"
#include "moloptionsdialog.h"
#include "settingsdialog.h"
#include "makeatomdialog.h"
#include "makelatticedialog.h"

#include <QtGui>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QApplication>
#include <QHeaderView>
#include <QWidget>
#include <QContextMenuEvent>

MainWindow::MainWindow(QMainWindow *parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    exeDir_=QDir::currentPath();

    createActions();

//    ui.statusBar->hide();

    ui.tabWidget->setEnabled(false);
    //the following are the formats of the common paths
//    projectName_ = "AcN";
//    workingDir_ = "E:/EPSR24/run/AcN/";
//    epsrBinDir_ = "E:/EPSR24/bin/";
//    atoFileName_ = "AcNbox.ato";
//    nPartials = 10;
    ui.plot1Button->setEnabled(false);
    ui.plot2Button->setEnabled(false);

    ui.fileMenu->setFocus();

    QRegExp numberDensityrx("^\\d*\\.?\\d*$");
    ui.numberDensityLineEdit->setValidator((new QRegExpValidator(numberDensityrx, this)));

    ui.normalisationComboBox->setCurrentIndex(0);
    ui.plotComboBox2->setCurrentIndex(2);

    QStringList atoheader;
    atoheader << "Component" << "Charge" << "# in box";
    ui.atoFileTable->setHorizontalHeaderLabels(atoheader);
    ui.atoFileTable->verticalHeader()->setVisible(false);
    ui.atoFileTable->horizontalHeader()->setVisible(true);

    plotDialog = 0;
//    createNewDialog = 0;
    molOptionsDialog = 0;
//    settingsDialog = 0;
    makeAtomDialog = 0;
    makeLatticeDialog = 0;

    readSettings();

    connect(ui.plot1, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(showPointToolTip1(QMouseEvent*)));
    connect(ui.plot2, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(showPointToolTip2(QMouseEvent*)));
    connect(ui.plot1, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(plotZoom1(QWheelEvent*)));
    connect(ui.plot2, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(plotZoom2(QWheelEvent*)));

}

void MainWindow::createActions()
{
    ui.newAct->setStatusTip(tr("Create a new EPSR Project"));
    connect(ui.newAct, SIGNAL(triggered()), this, SLOT(createNew()));

    ui.openAct->setStatusTip(tr("Open an existing EPSR Project"));
    connect(ui.openAct, SIGNAL(triggered()), this, SLOT(open()));

    ui.saveAsAct->setStatusTip(tr("Make a copy of the current EPSR Project"));
    connect(ui.saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    ui.saveAct->setStatusTip(tr("Save the current EPSR Project"));
    connect(ui.saveAct, SIGNAL(triggered()), this, SLOT(save()));

    ui.exitAct->setStatusTip(tr("Exit the application"));
    connect(ui.exitAct, SIGNAL(triggered()), this, SLOT(close()));

    ui.plotAct->setStatusTip(tr("Plot EPSR outputs"));
    connect(ui.plotAct, SIGNAL(triggered()), this, SLOT(plot()));

    ui.settingsAct->setStatusTip(tr("Change EPSRProject settings"));
    connect(ui.settingsAct, SIGNAL(triggered()), this, SLOT(settings()));

    ui.checkAct->setStatusTip(tr("Run EPSR once to check everything works"));
    connect(ui.checkAct, SIGNAL(triggered()), this, SLOT(runEPSRcheck()));

    ui.runAct->setStatusTip(tr("Run EPSR"));
    connect(ui.runAct, SIGNAL(triggered()), this, SLOT(runEPSR()));

    ui.stopAct->setStatusTip(tr("Run EPSR"));
    connect(ui.stopAct, SIGNAL(triggered()), this, SLOT(stopEPSR()));

    ui.deleteEPSRinpFileAct->setStatusTip(tr("Delete the existing EPSR .inp file"));
    connect(ui.deleteEPSRinpFileAct, SIGNAL(triggered()), this, SLOT(deleteEPSRinpFile()));

    ui.deleteBoxAtoFileAct->setStatusTip(tr("Delete the existing box .ato file"));
    connect(ui.deleteBoxAtoFileAct, SIGNAL(triggered()), this, SLOT(deleteBoxAtoFile()));
}

void MainWindow::onCustomContextMenu(const QPoint &point)
{
    QMenu contextMenu(tr("Context menu"), this);

//    QAction action1("Remove Data Point", this);
//    connect(&action1, SIGNAL(triggered()), this, SLOT(removeDataPoint()));
//    contextMenu.addAction(&action1);

//    contextMenu.exec(mapToGlobal(pos));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //stopEPSR();     WANT TO STOP EPSR HERE IF RUNNING BUT THIS IS NOT HOW TO DO IT!!!!!!!!!!
    event->accept();
}

void MainWindow::processStart()
{
    QProgressDialog progress("working...", "Cancel", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);

    for (int i = 0; i < 100; ++i)
    {
        progress.setValue(i);
        QThread::sleep(1);
        QApplication::processEvents();
        if(progress.wasCanceled())
            break;
    }
}

void MainWindow::processEnd()
{
//    ui.statusBar->hide();
}

void MainWindow::readSettings()
{
    QString settingsFile = exeDir_.path()+"/settings";
    settingsFile = QDir::toNativeSeparators(settingsFile);
    QFile file(settingsFile);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open settings file");
        msgBox.exec();
        return;
    }
    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    do
    {
        line = stream.readLine();
        dataLine = line.split("  ", QString::SkipEmptyParts);
        if (dataLine.count() != 0)
        {
            if (dataLine.at(0) == "EPSRdir")
            {
                currentDir = dataLine.at(1);
                epsrDir_ = dataLine.at(1);
            }
            if (dataLine.at(0) == "visualiser")
            {
                visualiserExe_ = dataLine.at(1);
                visualiserExe_ = QDir::toNativeSeparators(visualiserExe_);
            }
        }
    } while (!stream.atEnd());
    file.close();
}

void MainWindow::createNew()
{
    CreateNewDialog createNewDialog(this);

    createNewDialog.show();
    createNewDialog.raise();
    createNewDialog.activateWindow();

    newDialog = createNewDialog.exec();

    if (newDialog == CreateNewDialog::Accepted)
    {
        reset();

        //set file directories
        projectName_ = createNewDialog.getEPSRname();
        epsrDir_ = createNewDialog.getEPSRdir();
        epsrDir_ = QDir::toNativeSeparators(epsrDir_);
        workingDir_ = (epsrDir_+"/run/"+projectName_+"/");
        workingDir_ = QDir::toNativeSeparators(workingDir_);
        if (!QDir(workingDir_).exists())
        {
            QDir().mkdir(workingDir_);
        }
        printf("Current EPSR project name is %s\n", qPrintable(projectName_));
        printf("Current working directory is %s\n", qPrintable(workingDir_));
        QDir::setCurrent(workingDir_);
        epsrBinDir_ = (epsrDir_+"/bin/");
        epsrBinDir_ = QDir::toNativeSeparators(epsrBinDir_);
        printf("Current EPSR binaries directory is %s\n", qPrintable(epsrBinDir_));

        //make .EPSR.pro file
        QString epsrProFile = workingDir_+"/"+projectName_+".EPSR.pro";
        QFile file(epsrProFile);
        file.open(QFile::ReadWrite | QFile::Text);
        file.close();

        //copy important EPSR files into working directory
        QFile::copy(epsrDir_+"/startup/f0_WaasKirf.dat", workingDir_+"/f0_WaasKirf.dat");
        QFile::copy(epsrDir_+"/startup/plot_defaults.txt", workingDir_+"/plot_defaults.txt");
        QFile::copy(epsrDir_+"/startup/vanderWaalsRadii.txt", workingDir_+"/vanderWaalsRadii.txt");
        QFile::copy(epsrDir_+"/startup/gnubonds.txt", workingDir_+"/gnubonds.txt");
        QFile::copy(epsrDir_+"/startup/gnuatoms.txt", workingDir_+"/gnuatoms.txt");
        QFile::copy(epsrDir_+"/startup/system_commands.txt", workingDir_+"/system_commands.txt");
#ifdef _WIN32
        QFile::copy(epsrDir_+"/startup/epsr.bat", workingDir_+"/epsr.bat");
#else
        QFile::copy(epsrDir_+"/startup/epsr", workingDir_+"/epsr");
#endif

        //activate tabs
        ui.tabWidget->setEnabled(true);

        //activate menu options
        ui.saveAct->setEnabled(true);
        ui.saveAsAct->setEnabled(true);

        //activate buttons (in case disabled from previous project)
        ui.createMolFileButton->setEnabled(true);
        ui.molFileLoadButton->setEnabled(true);
        ui.createAtomButton->setEnabled(true);
        ui.createLatticeButton->setEnabled(true);
        ui.makeMolExtButton->setEnabled(true);
        ui.removeMolFileButton->setEnabled(true);
        ui.addLJRowAboveButton->setEnabled(true);
        ui.addLJRowBelowButton->setEnabled(true);
        ui.deleteLJRowButton->setEnabled(true);
        ui.mixatoButton->setEnabled(true);
        ui.addatoButton->setEnabled(true);
        ui.dataFileBrowseButton->setEnabled(true);
        ui.removeDataFileButton->setEnabled(true);

        //change window title to contain projectName
        this->setWindowTitle("EPSRgui: "+projectName_);
        ui.messagesLineEdit->setText("New project "+projectName_+" created");
    }
}

void MainWindow::reset()
{
    //clear tables and key names
    projectName_.clear();
    workingDir_.clear();
    epsrBinDir_.clear();

    ui.molFileList->clear();
    ui.molChargeLabel->clear();
    ui.molAtomTable->clearContents();
    ui.molBondTable->clearContents();
    ui.molAngleTable->clearContents();
    ui.molDihedralTable->clearContents();
    ui.molRotTable->clearContents();
    ui.molLJTable->clearContents();
    molFileName_.clear();

    ui.atoFileTable->clearContents();
    ui.numberDensityLineEdit->clear();
    ui.boxAtoLabel->clear();
    ui.boxAtoCharge->clear();
    ui.boxAtoMols->clear();
    ui.boxAtoLength->clear();
    ui.temperatureLineEdit->setText("300");
    ui.vibtempLineEdit->setText("65");
    ui.angtempLineEdit->setText("1");
    ui.dihtempLineEdit->setText("1");
    ui.atoAtomList->clear();
    ui.ecoredcoreTable->item(0,0)->setText("0");
    ui.ecoredcoreTable->item(0,1)->setText("1");
    atoFileName_.clear();
    baseFileName_.clear();

    ui.dataFileTable->clearContents();
    ui.atomWtsTable->clearContents();
    dataFileName_.clear();
    wtsBaseFileName_.clear();
    dataFileExt_.clear();

    ui.epsrInpFileName->clear();
    ui.inpSettingsTable->clearContents();
    ui.dataFileSettingsTable->clearContents();
    ui.pcofSettingsTable->clearContents();
    ui.minDistanceTable->clearContents();
    epsrInpFileName_.clear();

    //clear plots
    ui.plot1->clearGraphs();
    ui.plot1->clearItems();
    ui.plot1->clearPlottables();
    ui.plot1->replot();
    ui.plot2->clearGraphs();
    ui.plot2->clearItems();
    ui.plot2->clearPlottables();
    ui.plot2->replot();
}

void MainWindow::open()
{
    if (!epsrDir_.isEmpty())
    {
        currentDir=epsrDir_;
    }
    QString newFileName = QFileDialog::getOpenFileName(this, "Choose EPSR .pro file", currentDir.path(), tr(".EPSR.pro files (*.EPSR.pro)"));
    if (!newFileName.isEmpty())
    {
        //clear all tables
        reset();

        // set file directories
        projectName_ = QFileInfo(newFileName).baseName();
        printf("Current EPSR project name is %s\n", qPrintable(projectName_));
        workingDir_ = QFileInfo(newFileName).path()+"/";
        workingDir_ = QDir::toNativeSeparators(workingDir_);
        printf("Current working directory is %s\n", qPrintable(workingDir_));
        QDir::setCurrent(workingDir_);
        epsrDir_ = workingDir_.split("run",QString::SkipEmptyParts).at(0);
        epsrBinDir_ = (epsrDir_+"bin/");
        epsrBinDir_ = QDir::toNativeSeparators(epsrBinDir_);
        printf("Current EPSR binaries directory is %s\n", qPrintable(epsrBinDir_));

        //read in EPSR pro file
        QFile file(newFileName);
        if(!file.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open EPSR pro file");
            msgBox.exec();
            return;
        }
        QTextStream stream(&file);
        QString line;
        QStringList dataLine;
        dataLine.clear();

        dataFileList.clear();
        dataFileTypeList.clear();
        wtsFileList.clear();
        normalisationList.clear();
        ui.atoAtomList->clear();

        do
        {
            line = stream.readLine();
            dataLine = line.split(" ", QString::SkipEmptyParts);
            if (dataLine.count() != 0)
            {
                if (dataLine.at(0) == "mol")
                {
                    ui.molFileList->QListWidget::addItem(dataLine.at(1)+".mol");
                    nMolFiles = ui.molFileList->count();
                    ui.atoFileTable->setRowCount(nMolFiles);
                    QTableWidgetItem *item = new QTableWidgetItem(dataLine.at(1)+".ato");
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                    ui.atoFileTable->setItem(nMolFiles-1,0, item);
                    ui.atoFileTable->setItem(nMolFiles-1,2, new QTableWidgetItem(dataLine.at(2)));
                    ui.molFileList->setCurrentRow(nMolFiles-1);
                }
                if (dataLine.at(0) == "boxAtoFileName")
                {
                    atoFileName_ = dataLine.at(1);
                    printf("Box .ato filename is %s\n", qPrintable(atoFileName_));
                    ui.boxAtoLabel->setText(atoFileName_);
                    readAtoFileBoxDetails();
                    readAtoFileAtomPairs();
                    for (int n=0; n < atoAtomLabels.count(); ++n)
                    {
                        QListWidgetItem* item = new QListWidgetItem(atoAtomLabels.at(n));
                        item->setData(Qt::UserRole, n);
                        ui.atoAtomList->addItem(item);
                    }
                    checkBoxCharge();
                    ui.createMolFileButton->setDisabled(true);
                    ui.molFileLoadButton->setDisabled(true);
                    ui.createAtomButton->setDisabled(true);
                    ui.createLatticeButton->setDisabled(true);
                    ui.makeMolExtButton->setDisabled(true);
                    ui.removeMolFileButton->setDisabled(true);
                    ui.addLJRowAboveButton->setDisabled(true);
                    ui.addLJRowBelowButton->setDisabled(true);
                    ui.deleteLJRowButton->setDisabled(true);
                    ui.mixatoButton->setDisabled(true);
                    ui.addatoButton->setDisabled(true);
                }
                if (dataLine.at(0) == "numberDensity")
                {
                    ui.numberDensityLineEdit->setText(dataLine.at(1));
                }
                if (dataLine.at(0) == "temperature")
                {
                    ui.temperatureLineEdit->setText(dataLine.at(1));
                }
                if (dataLine.at(0) == "vibtemp")
                {
                    ui.vibtempLineEdit->setText(dataLine.at(1));
                }
                if (dataLine.at(0) == "angtemp")
                {
                    ui.angtempLineEdit->setText(dataLine.at(1));
                }
                if (dataLine.at(0) == "dihtemp")
                {
                    ui.dihtempLineEdit->setText(dataLine.at(1));
                }
                if (dataLine.at(0) == "ecoredcore")
                {
                    ui.ecoredcoreTable->setColumnCount(2);
                    ui.ecoredcoreTable->setRowCount(1);
                    ui.ecoredcoreTable->horizontalHeader()->setVisible(false);
                    ui.ecoredcoreTable->verticalHeader()->setVisible(false);
                    ui.ecoredcoreTable->setItem(0,0, new QTableWidgetItem(dataLine.at(1)));
                    ui.ecoredcoreTable->setItem(0,1, new QTableWidgetItem(dataLine.at(2)));
                }
                if (dataLine.at(0) == "fmoleIter")
                {
                    ui.fmoleLineEdit->setText(dataLine.at(1));
                }
                if (dataLine.at(0) == "data")
                {
                    dataFileTypeList.append(dataLine.at(1));
                    dataFileList.append(dataLine.at(2));
                    normalisationList.append(dataLine.at(3));
                }
                if (dataLine.at(0) == "wts")
                {
                     wtsFileList.append(dataLine.at(1));
                }
                if (dataLine.at(0) == "EPSRinp")
                {
                    epsrInpFileName_ = workingDir_+dataLine.at(1)+".EPSR.inp";
                    printf("EPSR inp filename is %s\n", qPrintable(epsrInpFileName_));
                    ui.epsrInpFileName->setText(epsrInpFileName_);
                    readEPSRinpFile();
                    updateInpFileTables();
                    readAtoFileAtomPairs();
                    readEPSRpcofFile();
                    updatePcofFileTables();
                    ui.plot1Button->setEnabled(true);
                    ui.plot2Button->setEnabled(true);
                    ui.dataFileBrowseButton->setDisabled(true);
                    ui.removeDataFileButton->setEnabled(false);
                }
                if (dataLine.at(0) == "dlputils")
                {
                    ui.dlputilsOutCheckBox->setChecked(true);
                }
            }
        } while (!stream.atEnd());
        file.close();

        // fill out wts table
        ui.dataFileTable->setColumnCount(4);
        QStringList datafileheader;
        datafileheader << "Data File" << "Data File Type" << "Normalisation" << "Wts File";
        ui.dataFileTable->setHorizontalHeaderLabels(datafileheader);
        ui.dataFileTable->verticalHeader()->setVisible(false);
        ui.dataFileTable->horizontalHeader()->setVisible(true);

        if (!dataFileList.isEmpty())
        {
            ui.dataFileTable->setRowCount(dataFileList.count());
            for (int i = 0; i < dataFileList.count(); i++)
            {
                QTableWidgetItem *itemdata = new QTableWidgetItem(dataFileList.at(i));
                itemdata->setFlags(itemdata->flags() & ~Qt::ItemIsEditable);
                ui.dataFileTable->setItem(i,0, itemdata);
                ui.dataFileTable->setItem(i,1, new QTableWidgetItem(dataFileTypeList.at(i)));
                QTableWidgetItem *itemnorm = new QTableWidgetItem(normalisationList.at(i));
                itemnorm->setFlags(itemnorm->flags() & ~Qt::ItemIsEditable);
                ui.dataFileTable->setItem(i,2, itemnorm);
                QTableWidgetItem *itemwts = new QTableWidgetItem(wtsFileList.at(i));
                itemwts->setFlags(itemwts->flags() & ~Qt::ItemIsEditable);
                ui.dataFileTable->setItem(i,3, itemwts);
            }
            ui.dataFileTable->setColumnWidth(0, 200);
            ui.dataFileTable->setColumnWidth(1, 90);
            ui.dataFileTable->setColumnWidth(2, 90);
            ui.dataFileTable->setColumnWidth(3, 200);

            ui.dataFileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
            ui.dataFileTable->setSelectionMode(QAbstractItemView::SingleSelection);
            ui.dataFileTable->setCurrentCell(dataFileList.count()-1,0);
        }
        if (!wtsFileList.isEmpty())
        for (int i = 0; i < dataFileList.count(); i++)
        {
            ui.dataFileTable->setItem(i,3, new QTableWidgetItem(wtsFileList.at(i)));
        }

        //activate tabs
        ui.tabWidget->setEnabled(true);

        //activate menu options
        ui.saveAct->setEnabled(true);
        ui.saveAsAct->setEnabled(true);

        //change window title to contain projectName
        this->setWindowTitle("EPSRProject: "+projectName_);

        //plot data if present
        if (!atoFileName_.isEmpty())
        {
            QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);
            if (QFile::exists(atoBaseFileName+".EPSR.u01") && !dataFileList.isEmpty())
            {
                plot1();
            }
            if (QFile::exists(atoBaseFileName+".EPSR.erg"))
            {
                plot2();
            }
        }
        ui.messagesLineEdit->setText("Project "+projectName_+" loaded");
    }
}

bool MainWindow::save()
{
    return saveFile();
}

bool MainWindow::saveAs()
{
    CreateNewDialog createNewDialog(this);

    createNewDialog.show();
    createNewDialog.raise();
    createNewDialog.activateWindow();

    newDialog = createNewDialog.exec();

    if (newDialog == CreateNewDialog::Accepted)
    {
        //set file directories
        QString projectNameCopy = createNewDialog.getEPSRname();
        QString epsrDirCopy = createNewDialog.getEPSRdir();
        QString workingDirCopy = (epsrDirCopy+"/run/"+projectNameCopy+"/");
        workingDirCopy = QDir::toNativeSeparators(workingDirCopy);
        QDir workingDir(workingDir_);

        //make new directory and make a list of everything in the current directory
        QDir().mkdir(workingDirCopy);
        QStringList fileList = workingDir.entryList(QDir::Files);

        //copy everything into new folder
        for (int i = 0; i < fileList.count(); i++)
        {
            QFile::copy(workingDir_+fileList.at(i), workingDirCopy+fileList.at(i));
        }

        //rename everything with original atoFileName_ in title
        QDir newDir(workingDirCopy);
        QStringList newfileList = newDir.entryList(QDir::Files);
        for (int i = 0; i < newfileList.count(); i++)
        {
            QString oldfilename = newfileList.at(i);
            QString newfilename = newfileList.at(i);
            newfilename.replace(projectName_+"box", projectNameCopy+"box");
            QFile::rename(workingDirCopy+oldfilename, workingDirCopy+newfilename);
        }

        //delete original .EPSR.pro file
        QString epsrProFile = workingDirCopy+projectName_+".EPSR.pro";
        QFile filedelete(epsrProFile);
        filedelete.remove();

        //make new .EPSR.pro file
        QString epsrProFileCopy = workingDirCopy+projectNameCopy+".EPSR.pro";
        QFile file(epsrProFileCopy);

        if(!file.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not save to "+projectNameCopy+".EPSR.pro");
            msgBox.exec();
        }

        QTextStream streamWrite(&file);

        //mol files and number of them in box
        if (ui.molFileList->count() != 0)
        {
            for (int i = 0; i < ui.molFileList->count(); i++)
            {
                streamWrite << "mol " << ui.molFileList->item(i)->text().split(".",QString::SkipEmptyParts).at(0)
                            << " " << ui.atoFileTable->item(i,2)->text() << "\n";
            }
        }

        //box ato file details
        if (!atoFileName_.isEmpty())
        {
            streamWrite << "boxAtoFileName " << projectNameCopy+"box.ato" << "\n";
            streamWrite << "numberDensity " << ui.numberDensityLineEdit->text() << "\n";
            streamWrite << "temperature " << ui.temperatureLineEdit->text() << "\n";
            streamWrite << "vibtemp " << ui.vibtempLineEdit->text() << "\n";
            streamWrite << "angtemp " << ui.angtempLineEdit->text() << "\n";
            streamWrite << "dihtemp " << ui.dihtempLineEdit->text() << "\n";
            streamWrite << "ecoredcore " << ui.ecoredcoreTable->item(0,0)->text() << " " << ui.ecoredcoreTable->item(0,1)->text() << "\n";

            //fmole iterations
            streamWrite << "fmoleIter " << ui.fmoleLineEdit->text() << "\n";
        }

        //data and wts files
        if (ui.dataFileTable->rowCount() != 0)
        {
            for (int i = 0; i < ui.dataFileTable->rowCount(); i++)
            {
                streamWrite << "data " << ui.dataFileTable->item(i,1)->text() << " " << ui.dataFileTable->item(i,0)->text() << " " << ui.dataFileTable->item(i,2)->text() << "\n";
            }
            for (int i = 0; i < wtsFileList.count(); i++)
            {
                streamWrite << "wts " << ui.dataFileTable->item(i,3)->text() << "\n";
            }
        }

        //change fnameato in NWTS.dat and XWTS.dat
        if (!wtsFileList.isEmpty())
        {
            for (int i = 0; i < wtsFileList.count(); i++)
            {
                QString wtsFileName;
                QString wtsFileType = wtsFileList.at(i).split(".",QString::SkipEmptyParts).at(1);
                if (wtsFileType == "XWTS")
                {
                    wtsFileName = wtsFileList.at(i).split(".",QString::SkipEmptyParts).at(0)+".XWTS.dat";
                }
                else
                {
                    wtsFileName = wtsFileList.at(i).split(".",QString::SkipEmptyParts).at(0)+".NWTS.dat";
                }

                QFile filewts(workingDirCopy+wtsFileName);
                if(!filewts.open(QFile::ReadWrite | QFile::Text))
                {
                    QMessageBox msgBox;
                    msgBox.setText("Could not open .wts file");
                    msgBox.exec();
                    return false;
                }

                QTextStream streamwts(&filewts);
                QString linewts;
                QStringList dataLinewts;
                dataLinewts.clear();
                QString originalwts;

                while (!streamwts.atEnd())
                {
                    linewts = streamwts.readLine();
                    dataLinewts = linewts.split("  ", QString::SkipEmptyParts);
                    originalwts.append(linewts+"\n");
                    if (dataLinewts.count()!=0)
                    {
                        if (dataLinewts.at(0) == "fnameato")
                        {
                            originalwts.remove(linewts+"\n");
                            originalwts.append("fnameato    "+projectNameCopy+"box.ato               Name of .ato file\n");
                        }
                    }
                }

                filewts.resize(0);
                streamwts << originalwts;
                filewts.close();
            }
        }

        //EPSR.inp and .pcof files
        if (!epsrInpFileName_.isEmpty())
        {
            streamWrite << "EPSRinp " << projectNameCopy+"box" << "\n";
        }

        file.close();

        //edit EPSR.inp file so first line, fnameato and fnamepcof are updated
        if (!epsrInpFileName_.isEmpty())
        {
            QFile fileRead(workingDir_+projectName_+"box.EPSR.inp");
            if(!fileRead.open(QFile::ReadOnly | QFile::Text))
            {
                QMessageBox msgBox;
                msgBox.setText("Could not open .EPSR.inp file to copy");
                msgBox.exec();
                return false;
            }

            QFile fileWrite(workingDirCopy+projectNameCopy+"box.EPSR.inp");
            if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
            {
                QMessageBox msgBox;
                msgBox.setText("Could not open new .EPSR.inp file to write to");
                msgBox.exec();
                return false;
            }

            QTextStream streamRead(&fileRead);
            QTextStream streamWrite(&fileWrite);
            QString line;
            QStringList dataLine;
            dataLine.clear();
            QString original;

            line = streamRead.readLine();
            streamWrite << projectNameCopy+"box.EPSR               Title of this file\n";

            while (!streamRead.atEnd())
            {
                line = streamRead.readLine();
                dataLine = line.split("   ", QString::SkipEmptyParts);
                original.append(line+"\n");
                if (dataLine.count()!=0)
                {
                    if (dataLine.at(0) == "fnameato")
                    {
                        original.remove(line+"\n");
                        original.append("fnameato    "+projectNameCopy+"box.ato               Name of .ato file\n");
                    }
                    if (dataLine.at(0) == "fnamepcof")
                    {
                        original.remove(line+"\n");
                        original.append("fnamepcof   "+projectNameCopy+"box.pcof               Name of potential coefficients file.\n");
                    }
                }
            }

            fileWrite.resize(0);
            streamWrite << original;
            fileWrite.close();
            fileRead.close();
        }

        printf("Copied EPSR project to %s\n", qPrintable(workingDirCopy));
        ui.messagesLineEdit->setText("Copied current EPSR project to "+workingDirCopy);
    }
    return false;
}

bool MainWindow::saveFile()
{
    QString saveFileName = workingDir_+projectName_+".EPSR.pro";
    QFile file(saveFileName);

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not save to "+projectName_+".EPSR.pro");
        msgBox.exec();
    }

    QTextStream streamWrite(&file);

    //mol files and number of them in box
    if (ui.molFileList->count() != 0)
    {
        for (int i = 0; i < ui.molFileList->count(); i++)
        {
            streamWrite << "mol " << ui.molFileList->item(i)->text().split(".",QString::SkipEmptyParts).at(0)
                        << " " << ui.atoFileTable->item(i,2)->text() << "\n";
        }
    }

    //box ato file details
    if (!atoFileName_.isEmpty())
    {
        streamWrite << "boxAtoFileName " << atoFileName_ << "\n";
        streamWrite << "numberDensity " << ui.numberDensityLineEdit->text() << "\n";
        streamWrite << "temperature " << ui.temperatureLineEdit->text() << "\n";
        streamWrite << "vibtemp " << ui.vibtempLineEdit->text() << "\n";
        streamWrite << "angtemp " << ui.angtempLineEdit->text() << "\n";
        streamWrite << "dihtemp " << ui.dihtempLineEdit->text() << "\n";
        streamWrite << "ecoredcore " << ui.ecoredcoreTable->item(0,0)->text() << " " << ui.ecoredcoreTable->item(0,1)->text() << "\n";

        //fmole iterations
        streamWrite << "fmoleIter " << ui.fmoleLineEdit->text() << "\n";
    }

    //data and wts files
    if (ui.dataFileTable->rowCount() != 0)
    {
        for (int i = 0; i < ui.dataFileTable->rowCount(); i++)
        {
            streamWrite << "data " << ui.dataFileTable->item(i,1)->text() << " " << ui.dataFileTable->item(i,0)->text() << " " << ui.dataFileTable->item(i,2)->text() << "\n";
        }
        for (int i = 0; i < wtsFileList.count(); i++)
        {
            streamWrite << "wts " << ui.dataFileTable->item(i,3)->text() << "\n";
        }
    }

    //EPSR.inp and .pcof files
    if (!epsrInpFileName_.isEmpty())
    {
        QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);
        streamWrite << "EPSRinp " << atoBaseFileName << "\n";
    }

    //Additional options
    if (ui.dlputilsOutCheckBox->isChecked())
    {
        streamWrite << "dlputils" << "\n";
    }

    file.close();

    printf("Current EPSR project settings saved to %s.EPSR.pro\n", qPrintable(projectName_));
    ui.messagesLineEdit->setText("Saved current EPSR project");
    return false;

    if (!epsrInpFileName_.isEmpty())
    {
        updateInpFile();
        updatePcofFile();
    }
}

void MainWindow::runEPSRcheck()
{
    updateInpFile();
    updatePcofFile();

    QDir::setCurrent(workingDir_);

    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);
    QProcess processrunEPSR;
    processrunEPSR.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processrunEPSR.start(epsrBinDir_+"epsr.exe", QStringList() << workingDir_ << "epsr" << atoBaseFileName);
#else
    processrunEPSR.start(epsrBinDir_+"epsr", QStringList() << workingDir_ << "epsr" << atoBaseFileName);
#endif
    if (!processrunEPSR.waitForStarted()) return;
    if (!processrunEPSR.waitForFinished(1800000)) return;

    ui.messagesLineEdit->setText("EPSR check finished");

    ui.plot1Button->setEnabled(true);
    ui.plot2Button->setEnabled(true);

    readEPSRinpFile();
    readEPSRpcofFile();
    updateInpFileTables();
    updatePcofFileTables();
}

void MainWindow::runEPSR()
{
    updateInpFile();
    updatePcofFile();

    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

#ifdef _WIN32
    QFile batFile(workingDir_+"run"+atoBaseFileName+".bat");
#else
    QFile batFile(workingDir_+"run"+atoBaseFileName+".sh");
#endif
    if(!batFile.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open script file.");
        msgBox.exec();;
    }

    //show EPSR is running
    ui.epsrRunningSign->setEnabled(true);

    //disable editing buttons while EPSR is running
    ui.updateMolFileButton->setDisabled(true);
    ui.updateAtoFileButton->setDisabled(true);
    ui.randomiseButton->setDisabled(true);
    ui.fmoleButton->setDisabled(true);
    ui.atoEPSRButton->setDisabled(true);
    ui.makeWtsButton->setDisabled(true);

    if (ui.dlputilsOutCheckBox->isChecked())
    {
        //get box side length and divide by 2 to get value for bat file
        double boxLength = ui.boxAtoLength->text().toDouble();
        double halfboxLength = boxLength/2;
        QString halfboxLengthStr = QString::number(halfboxLength);

        //run EPSR on loop with dlputils output line
        QTextStream stream(&batFile);
#ifdef _WIN32
        stream << "set EPSRbin=" << epsrBinDir_ << "\n"
                << "set EPSRrun=" << workingDir_ << "\n"
                << ":loop\n"
                << "%EPSRbin%epsr.exe " << workingDir_ << " epsr " << atoBaseFileName << "\n"
                << "%EPSRbin%writexyz.exe " << workingDir_ << " writexyz " << atoBaseFileName << " y 0 " << halfboxLengthStr << " " << halfboxLengthStr << " -" << halfboxLengthStr << " " << halfboxLengthStr << " 0 0 0 0" << "\n"
                << "if not exist %EPSRrun%killepsr ( goto loop ) else del %EPSRrun%killepsr\n";
#else
        stream << "export EPSRbin=" << epsrBinDir_ << "\n"
                << "export EPSRrun=" << workingDir_ << "\n"
                << "while :\n"
                << "do\n"
                << "  \"$EPSRbin\"'epsr' " << workingDir_ << " epsr " << atoBaseFileName << "\n"
                << "%\"$EPSRbin\"'writexyz' " << workingDir_ << " writexyz " << atoBaseFileName << " y 0 " << halfboxLengthStr << " " << halfboxLengthStr << " -" << halfboxLengthStr << " " << halfboxLengthStr << " 0 0 0 0" << "\n"
                << "  if ([ -e " << workingDir_ << " ])\n"
                << "  then break\n"
                << "  fi\n"
                << "done\n"
                << "rm -r " << workingDir_ << "killepsr\n";
#endif
        batFile.close();
    }

    else
    {
    //run EPSR on loop
    QTextStream stream(&batFile);
#ifdef _WIN32
    stream << "set EPSRbin=" << epsrBinDir_ << "\n"
            << "set EPSRrun=" << workingDir_ << "\n"
            << ":loop\n"
            << "\"$EPSRbin\"'epsr' " << workingDir_ << " epsr " << atoBaseFileName << "\n"
            << "if not exist \"$EPSRrun\"'killepsr' ( goto loop ) else del \"$EPSRrun\"'killepsr'\n";
#else
        stream << "export EPSRbin=" << epsrBinDir_ << "\n"
                << "export EPSRrun=" << workingDir_ << "\n"
                << "while :\n"
                << "do\n"
                << "  \"$EPSRbin\"'epsr' " << workingDir_ << " epsr " << atoBaseFileName << "\n"
                << "  if ([ -e " << workingDir_ << " ])\n"
                << "  then break\n"
                << "  fi\n"
                << "done\n"
                << "rm -r " << workingDir_ << "killepsr\n";
#endif
    batFile.close();
    }

    QDir::setCurrent(workingDir_);

    QProcess processrunEPSRscript;
    processrunEPSRscript.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processrunEPSRscript.startDetached("run"+atoBaseFileName+".bat");
#else
    processrunEPSRscript.startDetached("sh run"+atoBaseFileName+".sh");
#endif

    ui.messagesLineEdit->setText("EPSR is running in a separate window");

    //enable plotting as data files should now exist ************if this is clicked before files exist does program crash?
    ui.plot1Button->setEnabled(true);
    ui.plot2Button->setEnabled(true);
}

void MainWindow::stopEPSR()
{
    QFile file(workingDir_+"killepsr");
    if(!file.open(QFile::WriteOnly))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not stop EPSR script");
        msgBox.exec();
    }
    file.close();
    ui.messagesLineEdit->setText("EPSR will stop at the end of this iteration");

    connect(&epsrFinished_, SIGNAL(fileChanged(const QString &)), this, SLOT(enableButtons()));
    epsrFinished_.addPath(workingDir_+"killepsr");
}

void MainWindow::enableButtons()
{
    //turn off EPSR running sign
    ui.epsrRunningSign->setEnabled(false);

    //re-enable editing buttons
    ui.updateMolFileButton->setEnabled(true);
    ui.updateAtoFileButton->setEnabled(true);
    ui.randomiseButton->setEnabled(true);
    ui.fmoleButton->setEnabled(true);
    ui.atoEPSRButton->setEnabled(true);
    ui.makeWtsButton->setEnabled(true);

    ui.messagesLineEdit->setText("EPSR stopped");

    epsrFinished_.removePath(workingDir_+"killepsr");

    readEPSRinpFile();
    readEPSRpcofFile();
    updateInpFileTables();
    updatePcofFileTables();
}


void MainWindow::plot()
{
    if (!plotDialog)
    {
        plotDialog = new PlotDialog(this);
    }

    plotDialog->show();
    plotDialog->raise();
    plotDialog->activateWindow();
}

void MainWindow::settings()
{
    SettingsDialog settingsDialog(this);

    settingsDialog.show();
    settingsDialog.raise();
    settingsDialog.activateWindow();

    setDialog = settingsDialog.exec();

    if (setDialog == SettingsDialog::Accepted)
    {
        epsrDir_.clear();
        visualiserExe_.clear();
        readSettings();
    }
}

QString MainWindow::workingDir()
{
    return workingDir_;
}

QString MainWindow::atoFileName()
{
    return atoFileName_;
}

QDir MainWindow::exeDir()
{
    return exeDir_;
}

QString MainWindow::epsrDir()
{
    return epsrDir_;
}

void MainWindow::deleteEPSRinpFile()
{
    QFile file(epsrInpFileName_);
    if(file.exists() == true)
    {
        QMessageBox::StandardButton msgBox;
        msgBox  = QMessageBox::question(this, "Warning", "This will delete the existing EPSR.inp file.\nProceed?", QMessageBox::Ok|QMessageBox::Cancel);
        if (msgBox == QMessageBox::Cancel)
        {
            return;
        }
        else
        {
            file.remove();
            QFile epsrFile(epsrInpFileName_);
            epsrFile.remove();
            epsrInpFileName_.clear();
            ui.epsrInpFileName->clear();
            ui.inpSettingsTable->clearContents();
            ui.dataFileSettingsTable->clearContents();
            ui.pcofSettingsTable->clearContents();
            ui.minDistanceTable->clearContents();
            epsrInpFileName_.clear();

            //re-enable buttons
            ui.dataFileBrowseButton->setEnabled(true);
            ui.removeDataFileButton->setEnabled(true);
        }
    }
    return;
}

void MainWindow::deleteBoxAtoFile()
{
    QFile file(atoFileName_);
    if(file.exists() == true)
    {
        QMessageBox::StandardButton msgBox;
        msgBox  = QMessageBox::question(this, "Warning", "This will delete the existing box .ato file and any wts and EPSR.inp files.\nProceed?", QMessageBox::Ok|QMessageBox::Cancel);
        if (msgBox == QMessageBox::Cancel)
        {
            return;
        }
        else
        {
            //remove box ato file, clear name and re-initialise ato tab
            file.remove();
            atoFileName_.clear();
            ui.atoAtomList->clear();
            ui.boxAtoLabel->clear();
            ui.boxAtoCharge->clear();

            //remove wts files and clear data and wts tables
            ui.dataFileTable->clearContents();
            dataFileList.clear();
            wtsFileList.clear();
            ui.atomWtsTable->clearContents();
            QDir dir(workingDir_);
            dir.setNameFilters(QStringList() << "*.NWTS.dat" << "*.XWTS.dat" << "*.wts");
            dir.setFilter(QDir::Files);
            foreach(QString dirFile, dir.entryList())
            {
                dir.remove(dirFile);
            }

            //remove inp file and clear name if exists
            if (!epsrInpFileName_.isEmpty() == true)
            {
                QFile epsrFile(epsrInpFileName_);
                epsrFile.remove();
                epsrInpFileName_.clear();
                ui.epsrInpFileName->clear();
                ui.inpSettingsTable->clearContents();
                ui.dataFileSettingsTable->clearContents();
                ui.pcofSettingsTable->clearContents();
                ui.minDistanceTable->clearContents();
            }

            //re-enable buttons
            ui.createMolFileButton->setEnabled(true);
            ui.molFileLoadButton->setEnabled(true);
            ui.createAtomButton->setEnabled(true);
            ui.createLatticeButton->setEnabled(true);
            ui.makeMolExtButton->setEnabled(true);
            ui.removeMolFileButton->setEnabled(true);
            ui.addLJRowAboveButton->setEnabled(true);
            ui.addLJRowBelowButton->setEnabled(true);
            ui.deleteLJRowButton->setEnabled(true);
            ui.mixatoButton->setEnabled(true);
            ui.addatoButton->setEnabled(true);
            ui.dataFileBrowseButton->setEnabled(true);
            ui.removeDataFileButton->setEnabled(true);

            //clear plots
            ui.plot1->clearGraphs();
            ui.plot1->clearItems();
            ui.plot1->replot();
            ui.plot2->clearGraphs();
            ui.plot2->clearItems();
            ui.plot2->replot();

            printf("box .ato file deleted\n");
        }
    }
    return;
}
