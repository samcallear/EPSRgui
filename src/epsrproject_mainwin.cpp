#include "epsrproject.h"
#include "ui_epsrproject.h"
#include "plotdialog.h"
#include "createnewdialog.h"
#include "moloptionsdialog.h"
#include "settingsdialog.h"
#include "makeatomdialog.h"
#include "makelatticedialog.h"
#include "boxcompositiondialog.h"
#include "addatodialog.h"
#include "messagesdialog.h"

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
#include <QProcess>

MainWindow::MainWindow(QMainWindow *parent) : QMainWindow(parent), messagesDialog(this)
{
    ui.setupUi(this);

    exeDir_=QDir::currentPath();

    createActions();

    ui.tabWidget->setEnabled(false);
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

    molOptionsDialog = 0;
    makeAtomDialog = 0;
    makeLatticeDialog = 0;

    readSettings();

    connect(ui.plot1, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(showPointToolTip1(QMouseEvent*)));
    connect(ui.plot2, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(showPointToolTip2(QMouseEvent*)));
    connect(ui.plot1, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(plotZoom1(QWheelEvent*)));
    connect(ui.plot2, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(plotZoom2(QWheelEvent*)));

    connect(ui.setupOutTypeComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(getOutputType()));
    connect(ui.dlputilsOutCheckBox, SIGNAL(stateChanged(int)), this, SLOT(outputDlputils()));
    connect(&processEPSR_, SIGNAL(readyReadStandardOutput()), this, SLOT(outputfromEPSRprocessReady()));
    //also need to refresh() on adding text to messagesText_*****************************************************************************************
}

void MainWindow::createActions()
{
    ui.newAct->setStatusTip(tr("Create a new EPSR Project"));
    connect(ui.newAct, SIGNAL(triggered()), this, SLOT(createNew()));

    ui.openAct->setStatusTip(tr("Open an existing EPSR Project"));
    connect(ui.openAct, SIGNAL(triggered()), this, SLOT(open()));

    ui.saveAct->setStatusTip(tr("Save the current EPSR Project"));
    connect(ui.saveAct, SIGNAL(triggered()), this, SLOT(save()));

    ui.saveAsAct->setStatusTip(tr("Make a copy of the current EPSR Project"));
    connect(ui.saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    ui.saveCopyAct->setStatusTip(tr("Save the current EPSR Project"));
    connect(ui.saveCopyAct, SIGNAL(triggered()), this, SLOT(saveCopy()));

    ui.exitAct->setStatusTip(tr("Exit the application"));
    connect(ui.exitAct, SIGNAL(triggered()), this, SLOT(close()));

    ui.plotAct->setStatusTip(tr("Plot EPSR outputs"));
    connect(ui.plotAct, SIGNAL(triggered()), this, SLOT(plot()));

    ui.plotEPSRshellAct->setStatusTip(tr("Plot EPSR using EPSRshell"));
    connect(ui.plotEPSRshellAct, SIGNAL(triggered()), this, SLOT(plotEPSRshell()));

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

    ui.showMessagesAct->setStatusTip(tr("Show messages from EPSR"));
    connect(ui.showMessagesAct, SIGNAL(triggered()), this, SLOT(showMessages()));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //stopEPSR();     WANT TO STOP EPSR HERE IF RUNNING BUT THIS IS NOT HOW TO DO IT!!!!!!!!!!
    event->accept();
}

void MainWindow::processStart()
{
//    QProgressDialog progress("working...", "Cancel", 0, 100, this);
//    progress.setWindowModality(Qt::WindowModal);

//    for (int i = 0; i < 100; ++i)
//    {
//        progress.setValue(i);
//        QThread::sleep(1);
//        QApplication::processEvents();
//        if(progress.wasCanceled())
//            break;
//    }
}

void MainWindow::processEnd()
{

}

void MainWindow::readSettings()
{
    QString settingsFile = exeDir_.path()+"/settings";
    settingsFile = QDir::toNativeSeparators(settingsFile);
    QFile file(settingsFile);
    if(!file.exists())
    {
        return;
    }
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
        messageText_ += "Current EPSR project name is "+projectName_+"\n";
        messageText_ += "Current working directory is "+workingDir_+"\n";
        QDir::setCurrent(workingDir_);
        epsrBinDir_ = (epsrDir_+"/bin/");
        epsrBinDir_ = QDir::toNativeSeparators(epsrBinDir_);
        messageText_ += "Current EPSR binaries directory is "+epsrBinDir_+"\n";

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
#ifdef _WIN32
        QFile::copy(epsrDir_+"/startup/epsr.bat", workingDir_+"/epsr.bat");
        QFile::copy(epsrDir_+"/startup/system_commands.txt", workingDir_+"/system_commands.txt");
#else
        QFile::copy(epsrDir_+"/startup/epsr", workingDir_+"/epsr");
        QFile::copy(epsrDir_+"/startup/system_commands_linux.txt", workingDir_+"/system_commands.txt");
#endif

        //activate tabs
        ui.tabWidget->setEnabled(true);

        //activate menu options
        ui.saveAct->setEnabled(true);
        ui.saveAsAct->setEnabled(true);
        ui.saveCopyAct->setEnabled(true);
        ui.copyAct->setEnabled(true);
        ui.cutAct->setEnabled(true);
        ui.pasteAct->setEnabled(true);

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
        ui.mixatoButton->setEnabled(false);
        ui.addatoButton->setEnabled(false);
        ui.loadBoxButton->setEnabled(false);
        ui.randomiseButton->setEnabled(false);
        ui.viewAtoFileButton->setEnabled(false);
        ui.boxCompositionButton->setEnabled(false);
        ui.updateAtoFileButton->setEnabled(false);
        ui.fmoleButton->setEnabled(false);
        ui.atoEPSRButton->setEnabled(false);
        ui.dataFileBrowseButton->setEnabled(false);
        ui.removeDataFileButton->setEnabled(false);
        ui.makeWtsButton->setEnabled(false);
        ui.setupEPSRButton->setEnabled(false);
        ui.updateInpPcofFilesButton->setEnabled(false);
        ui.reloadEPSRinpButton->setEnabled(false);
        ui.setupOutButton->setEnabled(false);
        ui.applyOutputsButton->setEnabled(false);

        //Activate available outputs list
        getOutputType();

        //change window title to contain projectName
        this->setWindowTitle("EPSRgui: "+projectName_);
        messagesDialog.refreshMessages();
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
    ui.molAtomTable->setRowCount(0);
    ui.molBondTable->clearContents();
    ui.molBondTable->setRowCount(0);
    ui.molAngleTable->clearContents();
    ui.molAngleTable->setRowCount(0);
    ui.molDihedralTable->clearContents();
    ui.molDihedralTable->setRowCount(0);
    ui.molRotTable->clearContents();
    ui.molRotTable->setRowCount(0);
    ui.molLJTable->clearContents();
    ui.molLJTable->setRowCount(0);
    molFileName_.clear();

    ui.atoFileTable->clearContents();
    ui.atoFileTable->setRowCount(0);
    ui.numberDensityLineEdit->clear();
    ui.boxAtoLabel->clear();
    ui.boxAtoCharge->clear();
    ui.boxAtoMols->clear();
    ui.boxAtoLengthA->clear();
    ui.boxAtoLengthB->clear();
    ui.boxAtoLengthC->clear();
    ui.boxAtoAxisA->clear();
    ui.boxAtoAxisB->clear();
    ui.boxAtoAxisG->clear();
    ui.boxAtoVol->clear();
    ui.temperatureLineEdit->setText("300");
    ui.vibtempLineEdit->setText("65");
    ui.angtempLineEdit->setText("1");
    ui.dihtempLineEdit->setText("1");
    ui.atoTetherTable->clearContents();
    ui.atoTetherTable->setRowCount(0);
    ui.atoAtomList->clear();
    atoFileName_.clear();
    baseFileName_.clear();

    ui.dataFileTable->clearContents();
    ui.dataFileTable->setRowCount(0);
    ui.atomWtsTable->clearContents();
    ui.atomWtsTable->setRowCount(0);
    dataFileName_.clear();
    wtsBaseFileName_.clear();
    dataFileExt_.clear();

    ui.epsrInpFileName->clear();
    ui.inpSettingsTable->clearContents();
    ui.inpSettingsTable->setRowCount(0);
    ui.dataFileSettingsTable->clearContents();
    ui.dataFileSettingsTable->setRowCount(0);
    ui.pcofSettingsTable->clearContents();
    ui.pcofSettingsTable->setRowCount(0);
    ui.minDistanceTable->clearContents();
    ui.minDistanceTable->setRowCount(0);
    epsrInpFileName_.clear();

    //enable/disable buttons
    ui.molFileTabWidget->setEnabled(false);
    ui.createMolFileButton->setEnabled(true);
    ui.molFileLoadButton->setEnabled(true);
    ui.createAtomButton->setEnabled(true);
    ui.createLatticeButton->setEnabled(true);
    ui.viewMolFileButton->setEnabled(false);
    ui.removeMolFileButton->setEnabled(false);
    ui.updateMolFileButton->setEnabled(false);
    ui.makeMolExtButton->setEnabled(true);
    ui.addLJRowAboveButton->setEnabled(true);
    ui.addLJRowBelowButton->setEnabled(true);
    ui.deleteLJRowButton->setEnabled(true);
    ui.mixatoButton->setEnabled(false);
    ui.addatoButton->setEnabled(false);
    ui.loadBoxButton->setEnabled(true);
    ui.randomiseButton->setEnabled(false);
    ui.viewAtoFileButton->setEnabled(false);
    ui.boxCompositionButton->setEnabled(false);
    ui.updateAtoFileButton->setEnabled(false);
    ui.fmoleButton->setEnabled(false);
    ui.atoEPSRButton->setEnabled(false);
    ui.dataFileBrowseButton->setEnabled(false);
    ui.removeDataFileButton->setEnabled(false);
    ui.makeWtsButton->setEnabled(false);
    ui.setupEPSRButton->setEnabled(false);
    ui.updateInpPcofFilesButton->setEnabled(false);
    ui.reloadEPSRinpButton->setEnabled(false);
    ui.setupOutButton->setEnabled(false);
    ui.applyOutputsButton->setEnabled(false);
    ui.dlputilsOutCheckBox->setEnabled(false);
    ui.plot1Button->setEnabled(false);
    ui.plot2Button->setEnabled(false);

    //clear plots
    ui.plot1->clearGraphs();
    ui.plot1->clearItems();
    ui.plot1->clearPlottables();
    ui.plot1->replot();
    ui.plot2->clearGraphs();
    ui.plot2->clearItems();
    ui.plot2->clearPlottables();
    ui.plot2->replot();

    //disable menu actions
    ui.saveAct->setEnabled(false);
    ui.saveAsAct->setEnabled(false);
    ui.saveCopyAct->setEnabled(false);
    ui.copyAct->setEnabled(false);
    ui.cutAct->setEnabled(false);
    ui.pasteAct->setEnabled(false);
    ui.checkAct->setEnabled(false);
    ui.runAct->setEnabled(false);
    ui.stopAct->setEnabled(false);
    ui.plotAct->setEnabled(false);
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
        messageText_ += "Current EPSR project name is "+projectName_+"\n";
        workingDir_ = QFileInfo(newFileName).path()+"/";
        workingDir_ = QDir::toNativeSeparators(workingDir_);
        messageText_ += "Current working directory is "+workingDir_+"\n";
        QDir::setCurrent(workingDir_);
        epsrDir_ = workingDir_.split("run",QString::SkipEmptyParts).at(0);
        epsrBinDir_ = (epsrDir_+"bin/");
        epsrBinDir_ = QDir::toNativeSeparators(epsrBinDir_);
        messageText_ += "Current EPSR binaries directory is "+epsrBinDir_+"\n";

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
                    ui.molFileList->QListWidget::addItem(dataLine.at(1));
                    nMolFiles = ui.molFileList->count();
                    ui.atoFileTable->setRowCount(nMolFiles);
                    QStringList filename = dataLine.at(1).split(".", QString::SkipEmptyParts);
                    if (filename.at(1) == "ato")
                    {
                        ui.mixatoButton->setDisabled(true);
                        ui.loadBoxButton->setDisabled(true);
                    }
                    QTableWidgetItem *item = new QTableWidgetItem(filename.at(0)+".ato");
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                    ui.atoFileTable->setItem(nMolFiles-1,0, item);
                    ui.atoFileTable->setItem(nMolFiles-1,2, new QTableWidgetItem(dataLine.at(2)));
                    ui.molFileList->setCurrentRow(nMolFiles-1);
                    ui.viewMolFileButton->setEnabled(true);
                    ui.removeMolFileButton->setEnabled(true);
                    ui.molFileTabWidget->setEnabled(true);
                    ui.updateMolFileButton->setEnabled(true);
                    ui.mixatoButton->setEnabled(true);
                    ui.addatoButton->setEnabled(true);
                    ui.loadBoxButton->setEnabled(true);
                }
                if (dataLine.at(0) == "boxAtoFileName")
                {
                    atoFileName_ = dataLine.at(1);
                    messageText_ += "Box .ato filename is "+atoFileName_+"\n";
                    ui.boxAtoLabel->setText(atoFileName_);
                    readAtoFileAtomPairs();
                    readAtoFileBoxDetails();
                    for (int n=0; n < atoAtomLabels.count(); ++n)
                    {
                        QListWidgetItem* item = new QListWidgetItem(atoAtomLabels.at(n));
                        item->setData(Qt::UserRole, n);
                        ui.atoAtomList->addItem(item);
                    }
                    checkBoxCharge();
                    ui.randomiseButton->setEnabled(true);
                    ui.viewAtoFileButton->setEnabled(true);
                    ui.boxCompositionButton->setEnabled(true);
                    ui.updateAtoFileButton->setEnabled(true);
                    ui.fmoleButton->setEnabled(true);
                    ui.atoEPSRButton->setEnabled(true);
                    ui.dataFileBrowseButton->setEnabled(true);
                    ui.removeDataFileButton->setEnabled(true);
                    ui.deleteBoxAtoFileAct->setEnabled(true);
                }
                if (dataLine.at(0) == "data")
                {
                    dataFileTypeList.append(dataLine.at(1));
                    dataFileList.append(dataLine.at(2));
                    normalisationList.append(dataLine.at(3));
                    ui.makeWtsButton->setEnabled(true);
                }
                if (dataLine.at(0) == "wts")
                {
                     wtsFileList.append(dataLine.at(1));
                     ui.setupEPSRButton->setEnabled(true);
                }
                if (dataLine.at(0) == "EPSRinp")
                {
                    epsrInpFileName_ = dataLine.at(1)+".EPSR.inp";
                    messageText_ += "EPSR inp filename is "+epsrInpFileName_+"\n";
                    ui.epsrInpFileName->setText(epsrInpFileName_);
                    readEPSRinpFile();
                    updateInpFileTables();
                    readEPSRpcofFile();
                    updatePcofFileTables();
                    ui.updateInpPcofFilesButton->setEnabled(true);
                    ui.reloadEPSRinpButton->setEnabled(true);
                    ui.setupOutButton->setEnabled(true);
                    ui.applyOutputsButton->setEnabled(true);
                    ui.dlputilsOutCheckBox->setEnabled(true);
                    ui.deleteBoxAtoFileAct->setEnabled(true);
                    ui.plot1Button->setEnabled(true);
                    ui.plot2Button->setEnabled(true);
                    ui.dataFileBrowseButton->setDisabled(true);
                    ui.removeDataFileButton->setEnabled(false);
                    ui.checkAct->setEnabled(true);
                    ui.runAct->setEnabled(true);
                    ui.stopAct->setEnabled(true);
                    ui.plotAct->setEnabled(true);
                    ui.plotEPSRshellAct->setEnabled(true);
                    ui.plot1Button->setEnabled(true);
                    ui.plot2Button->setEnabled(true);
                    ui.deleteEPSRinpFileAct->setEnabled(true);
                    //Activate available outputs list
                    getOutputType();
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
        ui.saveCopyAct->setEnabled(true);
        ui.copyAct->setEnabled(true);
        ui.cutAct->setEnabled(true);
        ui.pasteAct->setEnabled(true);

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
        messagesDialog.refreshMessages();
        ui.messagesLineEdit->setText("Project "+projectName_+" loaded");
    }
}

bool MainWindow::save()
{
    return saveFile();
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
            streamWrite << "mol " << ui.molFileList->item(i)->text()
                        << " " << ui.atoFileTable->item(i,2)->text() << "\n";
        }
    }

    //box ato file details
    if (!atoFileName_.isEmpty())
    {
        streamWrite << "boxAtoFileName " << atoFileName_ << "\n";
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

    messageText_ += "Current EPSR project settings saved to "+projectName_+".EPSR.pro\n";
    messagesDialog.refreshMessages();
    ui.messagesLineEdit->setText("Saved current EPSR project");
    return false;

    if (!epsrInpFileName_.isEmpty())
    {
        updateInpFile();
        updatePcofFile();
    }
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
            ui.boxAtoLabel->setText(projectNameCopy+"box.ato");
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
            if (QFile::exists(workingDir_+"run"+projectName_+".bat"))
            {
                QFile::remove(workingDir_+"run"+projectName_+".bat");
            }
        }

        //dlputils
        if (ui.dlputilsOutCheckBox->isChecked())
        {
            streamWrite << "dlputils\n";
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
                ui.epsrInpFileName->setText(projectNameCopy+"box.EPSR.inp");
                updateInpFileTables();
            }

            fileWrite.resize(0);
            streamWrite << original;
            fileWrite.close();
            fileRead.close();
        }

        // set file directories etc              
        projectName_ = projectNameCopy;
        messageText_ += "Current EPSR project name is "+projectName_+"\n";
        workingDir_ = workingDirCopy;
        workingDir_ = QDir::toNativeSeparators(workingDir_);
        messageText_ += "Current working directory is "+workingDir_+"\n";
        QDir::setCurrent(workingDir_);
        epsrDir_ = epsrDirCopy;
        epsrBinDir_ = (epsrDir_+"/bin/");
        epsrBinDir_ = QDir::toNativeSeparators(epsrBinDir_);
        messageText_ += "Current EPSR binaries directory is "+epsrBinDir_+"\n";
        if (!atoFileName_.isEmpty())
        {
            atoFileName_ = projectNameCopy+"box.ato";
        }
        if (!epsrInpFileName_.isEmpty())
        {
            epsrInpFileName_ = projectName_+"box.EPSR.inp";
        }

        //ensure current settings are saved to .pro file
        saveFile();

        //change window title to contain projectName
        this->setWindowTitle("EPSRProject: "+projectName_);


        messagesDialog.refreshMessages();
        ui.messagesLineEdit->setText("EPSR project saved to "+workingDirCopy);
    }
    return false;
}

bool MainWindow::saveCopy()
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
            streamWrite << "ecoredcore " << ui.ecoreLineEdit->text() << " " << ui.dcoreLineEdit->text() << "\n";

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

        messageText_ += "Copied EPSR project to "+workingDirCopy+"\n";
        messagesDialog.refreshMessages();
        ui.messagesLineEdit->setText("Copied current EPSR project to "+workingDirCopy);
    }
    return false;
}

void MainWindow::runEPSRcheck()
{
    updateInpFile();
    updatePcofFile();

    //check everything required is present in the folder???*********************************************************************************************

    QDir::setCurrent(workingDir_);

    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

//    processEPSR_.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processEPSR_.start(epsrBinDir_+"epsr.exe", QStringList() << workingDir_ << "epsr" << atoBaseFileName);
#else
    processEPSR_.start(epsrBinDir_+"epsr", QStringList() << workingDir_ << "epsr" << atoBaseFileName);
#endif
    if (!processEPSR_.waitForStarted()) return;
    if (!processEPSR_.waitForFinished(1800000)) return;

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

    // if script file already exists, don't overwrite it
    if (!batFile.exists())
    {
        if(!batFile.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open script file.");
            msgBox.exec();
        }

        //write script file to run epsr
        QTextStream stream(&batFile);
    #ifdef _WIN32
        stream << "set EPSRbin=" << epsrBinDir_ << "\n"
                << "set EPSRrun=" << workingDir_ << "\n"
                << ":loop\n"
                << "%EPSRbin%epsr.exe " << workingDir_ << " epsr " << atoBaseFileName << "\n"
                << "if not exist %EPSRrun%killepsr ( goto loop ) else del %EPSRrun%killepsr\n";
    #else
            stream << "export EPSRbin=" << epsrBinDir_ << "\n"
                    << "export EPSRrun=" << workingDir_ << "\n"
                    << "while :\n"
                    << "do\n"
                    << "  \"$EPSRbin\"'epsr' " << workingDir_ << " epsr " << atoBaseFileName << "\n"
                    << "  if ([ -e " << workingDir_ << "killepsr ])\n"
                    << "  then break\n"
                    << "  fi\n"
                    << "done\n"
                    << "rm -r " << workingDir_ << "killepsr\n";
    #endif
        batFile.close();
    }

    //run EPSR on loop
    QDir::setCurrent(workingDir_);

    QProcess processrunEPSRscript;
    processrunEPSRscript.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
//    processrunEPSRscript.setProcessChannelMode(QProcess::ForwardedChannels);
    processrunEPSRscript.startDetached("run"+atoBaseFileName+".bat");
#else
//    processrunEPSRscript.startDetached("gnome-terminal -e \"bash -c \\\"sh "+workingDir_+"run"+atoBaseFileName+".sh"; exec bash\\\"\"");
    processrunEPSRscript.startDetached("sh run"+atoBaseFileName+".sh");
//    if (processrunEPSRscript.waitForStarted(-1))
//    {
//        while (processrunEPSRscript.waitForReadyRead(-1))
//        {
//            messageText_ +=processrunEPSRscript.readAllStandardOutput();
//        }
//    }
#endif

    //show EPSR is running
    ui.epsrRunningSign->setEnabled(true);

    //disable editing buttons while EPSR is running
    ui.updateMolFileButton->setDisabled(true);
    ui.updateAtoFileButton->setDisabled(true);
    ui.randomiseButton->setDisabled(true);
    ui.fmoleButton->setDisabled(true);
    ui.atoEPSRButton->setDisabled(true);
    ui.makeWtsButton->setDisabled(true);

    ui.createMolFileButton->setEnabled(false);
    ui.molFileLoadButton->setEnabled(false);
    ui.createAtomButton->setEnabled(false);
    ui.createLatticeButton->setEnabled(false);
    ui.makeMolExtButton->setEnabled(false);
    ui.removeMolFileButton->setEnabled(false);
    ui.addLJRowAboveButton->setEnabled(false);
    ui.addLJRowBelowButton->setEnabled(false);
    ui.deleteLJRowButton->setEnabled(false);
    ui.mixatoButton->setEnabled(false);
    ui.addatoButton->setEnabled(false);
    ui.dataFileBrowseButton->setEnabled(false);
    ui.removeDataFileButton->setEnabled(false);

    ui.messagesLineEdit->setText("EPSR is running in a separate window");

    //enable plotting as data files should now exist ************if this is clicked before files exist does program crash?
    ui.inpSettingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.dataFileSettingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.pcofSettingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.minDistanceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
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

    ui.inpSettingsTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui.dataFileSettingsTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui.pcofSettingsTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui.minDistanceTable->setEditTriggers(QAbstractItemView::AllEditTriggers);

    ui.messagesLineEdit->setText("EPSR stopped");

    epsrFinished_.removePath(workingDir_+"killepsr");

    readEPSRinpFile();
    readEPSRpcofFile();
    updateInpFileTables();
    updatePcofFileTables();
}


void MainWindow::plot()
{
    PlotDialog plotDialog(this);

    plotDialog.show();
    plotDialog.raise();
    plotDialog.activateWindow();
    plotDialog.exec();
}

void MainWindow::plotEPSRshell()
{
    QDir::setCurrent(workingDir_);
    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

#ifdef _WIN32
    QFile batFile(workingDir_+"gnuplot"+atoBaseFileName+".bat");
#else
    QFile batFile(workingDir_+"gnuplot"+atoBaseFileName+".sh");
#endif
    if(!batFile.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open script file.");
        msgBox.exec();;
    }

    QString gnuBinDir = epsrBinDir_+"gnuplot/bin";      //note this is without the last "/"
    gnuBinDir = QDir::toNativeSeparators(gnuBinDir);

    QTextStream stream(&batFile);
#ifdef _WIN32
    stream << "set EPSRbin=" << epsrBinDir_ << "\n"
            << "set EPSRrun=" << workingDir_ << "\n"
            << "set EPSRgnu=" << gnuBinDir << "\n"
            << "%EPSRbin%plot.exe " << workingDir_ << " plot\n";
#else
    stream << "export EPSRbin=" << epsrBinDir_ << "\n"
            << "export EPSRrun=" << workingDir_ << "\n"
            << "export EPSRgnu=" << gnuBinDir << "\n"
            << "  \"$EPSRbin\"'plot' " << workingDir_ << " plot\n";
  #endif
    batFile.close();

    QDir::setCurrent(workingDir_);

    QProcess processrunEPSRplot;
    processrunEPSRplot.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processrunEPSRplot.startDetached("gnuplot"+atoBaseFileName+".bat");
#else
    processrunEPSRplot.startDetached("sh gnuplot"+atoBaseFileName+".sh");
#endif

    ui.messagesLineEdit->setText("started plot routine within EPSRshell");
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

QStringList MainWindow::atomLabels()
{
    return atoAtomLabels;
}

QVector<int> MainWindow::numberOfEachAtomLabel()
{
    return numberAtomLabels;
}

QStringList MainWindow::listAtoFiles()
{
    atoFileList.clear();
    for (int i = 0; i < ui.atoFileTable->rowCount(); i++)
    {
        atoFileList.append(ui.atoFileTable->item(i,0)->text());
    }
    return atoFileList;
}

QByteArray MainWindow::messageText()
{
    return messageText_;
}



void MainWindow::deleteEPSRinpFile()
{
    QFile file(workingDir_+epsrInpFileName_);
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
            QFile epsrFile(workingDir_+epsrInpFileName_);
            epsrFile.remove();
            epsrInpFileName_.clear();
            ui.epsrInpFileName->clear();
            ui.inpSettingsTable->clearContents();
            ui.inpSettingsTable->setRowCount(0);
            ui.dataFileSettingsTable->clearContents();
            ui.dataFileSettingsTable->setRowCount(0);
            ui.pcofSettingsTable->clearContents();
            ui.pcofSettingsTable->setRowCount(0);
            ui.minDistanceTable->clearContents();
            ui.minDistanceTable->setRowCount(0);
            epsrInpFileName_.clear();

            //clear plots
            ui.plot1->clearGraphs();
            ui.plot1->clearItems();
            ui.plot1->replot();
            ui.plot2->clearGraphs();
            ui.plot2->clearItems();
            ui.plot2->replot();

            //enable/disable buttons
            ui.runAct->setEnabled(false);
            ui.stopAct->setEnabled(false);
            ui.plotAct->setEnabled(false);
            ui.plot1Button->setEnabled(false);
            ui.plot2Button->setEnabled(false);
            ui.plotEPSRshellAct->setEnabled(false);
            ui.dataFileBrowseButton->setEnabled(true);
            ui.removeDataFileButton->setEnabled(true);
            ui.updateInpPcofFilesButton->setEnabled(false);
            ui.reloadEPSRinpButton->setEnabled(false);
            ui.setupOutButton->setEnabled(false);
            ui.applyOutputsButton->setEnabled(false);
            ui.dlputilsOutCheckBox->setEnabled(false);
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
            ui.boxAtoAxisA->clear();
            ui.boxAtoAxisB->clear();
            ui.boxAtoAxisG->clear();
            ui.boxAtoLengthA->clear();
            ui.boxAtoLengthB->clear();
            ui.boxAtoLengthC->clear();
            ui.boxAtoMols->clear();
            ui.boxAtoVol->clear();
            ui.boxAtoAtoms->clear();
            ui.atoTetherTable->clearContents();
            ui.atoTetherTable->setRowCount(0);
            ui.atoTetherTolLineEdit->setText(0);
            ui.numberDensityLineEdit->clear();

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
                QFile epsrFile(workingDir_+epsrInpFileName_);
                epsrFile.remove();
                epsrInpFileName_.clear();
                ui.epsrInpFileName->clear();
                ui.inpSettingsTable->clearContents();
                ui.dataFileSettingsTable->clearContents();
                ui.pcofSettingsTable->clearContents();
                ui.minDistanceTable->clearContents();
            }

            //enable/disable buttons
            ui.randomiseButton->setEnabled(false);
            ui.viewAtoFileButton->setEnabled(false);
            ui.boxCompositionButton->setEnabled(false);
            ui.updateAtoFileButton->setEnabled(false);
            ui.fmoleButton->setEnabled(false);
            ui.atoEPSRButton->setEnabled(false);
            ui.dataFileBrowseButton->setEnabled(false);
            ui.removeDataFileButton->setEnabled(false);
            ui.makeWtsButton->setEnabled(false);
            ui.setupEPSRButton->setEnabled(false);
            ui.updateInpPcofFilesButton->setEnabled(false);
            ui.reloadEPSRinpButton->setEnabled(false);
            ui.setupOutButton->setEnabled(false);
            ui.applyOutputsButton->setEnabled(false);
            ui.dlputilsOutCheckBox->setEnabled(false);

            //clear plots
            ui.plot1->clearGraphs();
            ui.plot1->clearItems();
            ui.plot1->replot();
            ui.plot2->clearGraphs();
            ui.plot2->clearItems();
            ui.plot2->replot();

            messageText_ += "box .ato file deleted\n";
            messagesDialog.refreshMessages();
        }
    }
    return;
}

void MainWindow::showMessages()
{
    messagesDialog.show();
    messagesDialog.raise();
    messagesDialog.activateWindow();
    messagesDialog.exec();
}

void MainWindow::outputfromEPSRprocessReady()
{
    messageText_ += processEPSR_.readAllStandardOutput();
    messagesDialog.refreshMessages();
}
