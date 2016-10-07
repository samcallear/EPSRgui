#include "setupoutputdialog.h"
#include "epsrproject.h"

#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>
#include <QString>

SetupOutputDialog::SetupOutputDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    ui.calcTabWidget->setEnabled(false);
    ui.outputSettingsTable->clear();
    ui.outputSettingsTable->setRowCount(0);
    ui.outputSettingsTableTab1->clear();
    ui.outputSettingsTableTab1->setRowCount(0);

    mainWindow_ = parent;

    workingDir_ = mainWindow_->workingDir();
    epsrBinDir_ = mainWindow_->epsrDir()+"bin/";
    epsrBinDir_ = QDir::toNativeSeparators(epsrBinDir_);

    atoFileName_ = mainWindow_->atoFileName();
    ui.atoFileLabel->setText(atoFileName_);
    outputFileName_ = mainWindow_->outputFileName();
    outputSetupFileType_ = mainWindow_->outputSetupFileType();
    outputFileExt_ = mainWindow_->outputFileExt();

    this->setWindowTitle("Setup Output Routine: "+outputSetupFileType_+" "+outputFileName_);

    QFileInfo fi(workingDir_+outputFileName_+outputFileExt_);
    if (!fi.exists())
    {
        makeOutputSetupFile();
    }
    readOutputSetupFile();
    updateTables();

    connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(writeOutputFile()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

}

void SetupOutputDialog::makeOutputSetupFile()
{
    QDir::setCurrent(workingDir_);

    QProcess processSetupOutput;
    processSetupOutput.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processSetupOutput.start(epsrBinDir_+"upset.exe", QStringList() << workingDir_ << "upset" << outputSetupFileType_ << outputFileName_);
#else
    processSetupOutput.start(epsrBinDir_+"upset", QStringList() << workingDir_ << "upset" << outputSetupFileType_ << outputFileName_);
#endif
    if (!processSetupOutput.waitForStarted()) return;

    processSetupOutput.write(qPrintable("fnameato "+atoFileName_+".ato\n"));
    processSetupOutput.write("e\n");
    processSetupOutput.write("\n");

    if (!processSetupOutput.waitForFinished(60000)) return;
}

void SetupOutputDialog::readOutputSetupFile()
{
    QFile file(workingDir_+outputFileName_+outputFileExt_);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open setup output file.");
        msgBox.exec();
        return;
    }

    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    outputKeywords.clear();
    outputValues.clear();
    outputDescriptions.clear();
    QRegExp rx("^.{12}");

    line = stream.readLine(); //title line
    line = stream.readLine(); //fnameato line

    //read in first group of settings
    do {
        line = stream.readLine();
        QString start = line;
        start.truncate(12);
        outputKeywords.append(start);
        if (line.count() > 1)
        {
            dataLine = line.split("               ", QString::SkipEmptyParts);
            QString firstPart = dataLine.at(0);
            outputValues.append(firstPart.split(rx, QString::SkipEmptyParts));
            outputDescriptions.append(dataLine.at(1));
        }
        else
        {
            outputValues.append(" ");
            outputDescriptions.append(" ");
        }
    } while (line != "q");

    file.close();




//    line = stream.readLine(); //blank line
//    QString subHeader;
//    line = stream.readLine(); //sub header line
//    subHeader = line.split(" ", QString::SkipEmptyParts).at(0);
//    int numSubHeaders = 0;
//    line = stream.readLine(); //blank line

//    do {
//        line = stream.readLine();

//    } while (line != "q");

    //open output file and read in to table
    //for coord, chains, triangles ndist gives number of distribtuions to be calculated
    //for plot3djmol, nsphere gives number of speheres at centre of plot
    //for sharm, sdf, only one setup is calculated

    //format different for plot2d and plot3d and need to define sharm file to be associated first, only 1 setup 'calculated'

}

void SetupOutputDialog::updateTables()
{
    int entries = 0;
    for (int i = 0; i < outputKeywords.count(); i++)
    {
        if (outputKeywords.at(i).contains("ndist") || outputKeywords.at(i).contains("nsphere") || outputKeywords.at(i).contains("q"))
        {
            break;
        }
        entries++;
    }

    ui.outputSettingsTable->clear();
    ui.outputSettingsTable->setColumnWidth(1,180);
    ui.outputSettingsTable->setColumnWidth(2,500);
    ui.outputSettingsTable->setRowCount(entries);
    ui.outputSettingsTable->horizontalHeader()->setVisible(true);
    QStringList header;
    header << "Keyword" << "Value" << "Description";
    ui.outputSettingsTable->setHorizontalHeaderLabels(header);
    ui.outputSettingsTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < entries; i++)
    {
        QTableWidgetItem *itemkeyword = new QTableWidgetItem(outputKeywords.at(i));
        itemkeyword->setFlags(itemkeyword->flags() & ~Qt::ItemIsEditable);
        ui.outputSettingsTable->setItem(i,0, itemkeyword);
        ui.outputSettingsTable->setItem(i,1, new QTableWidgetItem(outputValues.at(i)));
        QTableWidgetItem *itemdescrip = new QTableWidgetItem(outputDescriptions.at(i));
        itemdescrip->setFlags(itemdescrip->flags() & ~Qt::ItemIsEditable);
        ui.outputSettingsTable->setItem(i,2, itemdescrip);
    }
    ui.outputSettingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.outputSettingsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.outputSettingsTable->setCurrentCell(entries-1,0);

    if (outputKeywords.count() != entries+1)
    {
        QString subHeader = outputKeywords.at(entries+2); // this is the correct line to get the subheader and # e.g. chains 1
        subHeader = subHeader.split(" ", QString::SkipEmptyParts).at(0); //gets just the subheader, not the number e.g. chains
        int subEntries = outputValues.at(entries).toInt(); //this gets the number of subheaders
        //find number of lines in each subsection
        int linesPerEntry = 0;
        for (int i = entries+4; i < outputKeywords.count(); i++)
        {
            if (outputKeywords.at(i).contains(subHeader) || outputKeywords.at(i).contains("q"))
            {
                break;
            }
            linesPerEntry++;
        }

        ui.calcTabWidget->setEnabled(true);
        ui.outputSettingsTableTab1->clear();
        ui.outputSettingsTableTab1->setColumnWidth(1,180);
        ui.outputSettingsTableTab1->setColumnWidth(2,500);
        ui.outputSettingsTableTab1->setRowCount(linesPerEntry);
        ui.outputSettingsTableTab1->horizontalHeader()->setVisible(true);
        QStringList header;
        header << "Keyword" << "Value" << "Description";
        ui.outputSettingsTableTab1->setHorizontalHeaderLabels(header);
        ui.outputSettingsTableTab1->verticalHeader()->setVisible(false);
        //**********************THIS ONLY WORKS FOR A SINGLE ENTRY PER FILE!!!!!!*************************************************************
//        for (int j = 1; j < subEntries+1; j++)
//        {
//            ui.calcTabWidget->addTab(newTab, tr(j));
//            QTableWidget *outputSettingsEntryTable = new QTableWidget();
//            ui.calcTabWidget->
            for (int i = entries+4; i < entries+4+linesPerEntry; i++)
            {
                QTableWidgetItem *itemkeyword = new QTableWidgetItem(outputKeywords.at(i));
                itemkeyword->setFlags(itemkeyword->flags() & ~Qt::ItemIsEditable);
                ui.outputSettingsTableTab1->setItem(i-entries-4,0, itemkeyword);
                ui.outputSettingsTableTab1->setItem(i-entries-4,1, new QTableWidgetItem(outputValues.at(i)));
                QTableWidgetItem *itemdescrip = new QTableWidgetItem(outputDescriptions.at(i));
                itemdescrip->setFlags(itemdescrip->flags() & ~Qt::ItemIsEditable);
                ui.outputSettingsTableTab1->setItem(i-entries-4,2, itemdescrip);
            }
//            entries = entries+4+linesPerEntry+3;
//        }

        ui.outputSettingsTableTab1->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui.outputSettingsTableTab1->setSelectionMode(QAbstractItemView::SingleSelection);
        ui.outputSettingsTableTab1->setCurrentCell(linesPerEntry-1,0);
    }



}

void SetupOutputDialog::on_saveButton_clicked(bool checked)
{

}

void SetupOutputDialog::writeOutputFile()
{
    //read from table
    //write to file in correct format

    accept();
}

void SetupOutputDialog::on_cancelButton_clicked(bool checked)
{

}

void SetupOutputDialog::on_addTabButton_clicked(bool checked)
{

}

void SetupOutputDialog::on_removeTabButton_clicked(bool checked)
{

}

void SetupOutputDialog::on_deleteOutputButton_clicked(bool checked)
{

}
