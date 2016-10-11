#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>
#include <QString>

#include "setupoutputdialog.h"
#include "epsrproject.h"

SetupOutputDialog::SetupOutputDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    ui.calcGroupBox->setEnabled(false);
    ui.outputSettingsTable->clear();
    ui.outputSettingsTable->setRowCount(0);
    ui.calcSettingsTable->clear();
    ui.calcSettingsTable->setRowCount(0);

    mainWindow_ = parent;

    workingDir_ = mainWindow_->workingDir();
    epsrBinDir_ = mainWindow_->epsrDir()+"bin/";
    epsrBinDir_ = QDir::toNativeSeparators(epsrBinDir_);

    atoFileName_ = mainWindow_->atoFileName();
    ui.atoFileLabel->setText(atoFileName_);
    outputFileName_ = mainWindow_->outputFileName();
    outputSetupFileType_ = mainWindow_->outputSetupFileType();
    outputFileExt_ = mainWindow_->outputFileExt();

    settingEntries_ = 0;
    linesPerEntry_ = 0;
    entry_ = 0;
    outputKeywords.clear();
    outputValues.clear();
    outputDescriptions.clear();

    this->setWindowTitle("Setup Output Routine: "+outputSetupFileType_+" "+outputFileName_);

    connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(writeOutputFile()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QFileInfo fi(workingDir_+outputFileName_+outputFileExt_);
    if (!fi.exists())
    {
        makeOutputSetupFile();
    }
    readOutputSetupFile();
    updateTables();
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

    processSetupOutput.write(qPrintable("fnameato "+atoFileName_+"\n"));
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

//    outputcalcs.clear();
    outputKeywords.clear();
    outputValues.clear();
    outputDescriptions.clear();
    QRegExp rx("^.{12}");

    line = stream.readLine(); //title line
    line = stream.readLine(); //fnameato line

    //read in first group of settings
//    do {
//        line = stream.readLine();
//        if (line == "q") break;
//        else
//        if (line.isEmpty())
//        {
//            line = stream.readLine(); //read header line
//            header_ = line.split(" ", QString::SkipEmptyParts).at(0);
//            outputcalcs.append(OutputCalcs());
//            line = stream.readLine(); //read next blank line

//            do {
//               line = stream.readLine(); //read calculation lines
//               QString start = line;
//               start.truncate(12);
//               outputcalcs.last().keywords=start;
//               dataLine = line.split("               ", QString::SkipEmptyParts);
//               QString firstPart = dataLine.at(0);
//               outputcalcs.last().values=(firstPart.split(rx, QString::SkipEmptyParts)).at(1);
//               outputcalcs.last().descriptions=dataLine.at(1);
//            } while (!line.isEmpty());
//        }
//        else
//        {
//            QString start = line;
//            start.truncate(12);
//            outputKeywords.append(start);
//            dataLine = line.split("               ", QString::SkipEmptyParts);
//            QString firstPart = dataLine.at(0);
//            outputValues.append(firstPart.split(rx, QString::SkipEmptyParts));
//            outputDescriptions.append(dataLine.at(1));
//        }
//    } while (line != "q");

    do {
        line = stream.readLine();
        QString start = line;
        start.truncate(12);
        outputKeywords.append(start);
        dataLine = line.split("               ", QString::SkipEmptyParts);
        if (dataLine.count() == 0)
        {
            outputValues.append(" ");
            outputDescriptions.append(" ");
        }
        else
        {
            QString firstPart = dataLine.at(0);
            outputValues.append(firstPart.split(rx, QString::SkipEmptyParts).at(0));
            outputDescriptions.append(dataLine.at(1));
        }
    } while (!stream.atEnd());
    file.close();

    //remove the "q" line
    outputKeywords.removeLast();
    outputValues.removeLast();
    outputDescriptions.removeLast();


    //for plot3djmol, nsphere gives number of speheres at centre of plot
    //for sharm, sdf, only one setup is calculated
    //format different for plot2d and plot3d and need to define sharm file to be associated first, only 1 setup 'calculated'
}

void SetupOutputDialog::updateTables()
{
    settingEntries_ = 0; //this is the number of entries that aren't part of calculations, and doesn't include the title line, the fnameato line or the nXXX line if present
    for (int i = 0; i < outputKeywords.count(); i++)
    {
        if (outputKeywords.at(i).contains("ndist") || outputKeywords.at(i).contains("nsphere"))
        {
            break;
        }
        settingEntries_++;
    }

    ui.outputSettingsTable->clear();
    ui.outputSettingsTable->setColumnWidth(1,180);
    ui.outputSettingsTable->setColumnWidth(2,600);
    ui.outputSettingsTable->setRowCount(settingEntries_);
    ui.outputSettingsTable->horizontalHeader()->setVisible(true);
    QStringList header;
    header << "Keyword" << "Value" << "Description";
    ui.outputSettingsTable->setHorizontalHeaderLabels(header);
    ui.outputSettingsTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < settingEntries_; i++)
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
    ui.outputSettingsTable->setCurrentCell(settingEntries_-1,0);

//    const int Ncalcs = outputcalcs.count();
//    if (Ncalcs > 0)
    if (outputKeywords.count() > settingEntries_)
    {
        int subEntries = outputValues.at(settingEntries_).toInt(); //this gets the number of subheaders
        //find number of lines in each subsection
//        linesPerEntry_ = outputcalcs.at(0).keywords.count();
        linesPerEntry_ = 0;
        for (int i = settingEntries_+4; i < outputKeywords.count(); i++)
        {
            if (outputKeywords.at(i).isEmpty() || outputKeywords.at(i).contains("q"))
            {
                break;
            }
            linesPerEntry_++;
        }

        ui.calcGroupBox->setEnabled(true);

//        for (int i = 0; i < Ncalcs; i++)
        for (int i = 0; i < subEntries; i++)
        {
            ui.calcsList->addItem(QString::number(i+1)); //change this to be an atom or something more useful?
        }

        ui.calcsList->setCurrentRow(subEntries-1);
        entry_ = ui.calcsList->currentRow();
//        showEntryDetails();
    }
}

void SetupOutputDialog::on_calcsList_itemSelectionChanged()
{
    //first copy the values from the table to the stringlists
    if (ui.calcSettingsTable->rowCount() != 0)
    {
        int line = settingEntries_+4+((linesPerEntry_+3)*entry_);
        for (int i = 0; i < linesPerEntry_; i++)
        {
            outputValues.replace(line, ui.calcSettingsTable->item(i,1)->text());
            line++;
        }
    }

    entry_ = ui.calcsList->currentRow();
    if (entry_ > -1)
    {
        ui.calcSettingsTable->clear();
        ui.calcSettingsTable->setColumnWidth(1,180);
        ui.calcSettingsTable->setColumnWidth(2,300);
        ui.calcSettingsTable->setRowCount(linesPerEntry_);
        ui.calcSettingsTable->horizontalHeader()->setVisible(true);
        QStringList header;
        header << "Keyword" << "Value" << "Description";
        ui.calcSettingsTable->setHorizontalHeaderLabels(header);
        ui.calcSettingsTable->verticalHeader()->setVisible(false);

        int line = settingEntries_+4+((linesPerEntry_+3)*entry_);
        for (int i = 0; i < linesPerEntry_; i++)
        {
            QTableWidgetItem *itemkeyword = new QTableWidgetItem(outputKeywords.at(line));
            itemkeyword->setFlags(itemkeyword->flags() & ~Qt::ItemIsEditable);
            ui.calcSettingsTable->setItem(i,0, itemkeyword);
            ui.calcSettingsTable->setItem(i,1, new QTableWidgetItem(outputValues.at(line)));
            QTableWidgetItem *itemdescrip = new QTableWidgetItem(outputDescriptions.at(line));
            itemdescrip->setFlags(itemdescrip->flags() & ~Qt::ItemIsEditable);
            ui.calcSettingsTable->setItem(i,2, itemdescrip);
            line++;
        }

        ui.calcSettingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui.calcSettingsTable->setSelectionMode(QAbstractItemView::SingleSelection);
        ui.calcSettingsTable->setCurrentCell(0,0);
    }
}

void SetupOutputDialog::on_saveButton_clicked(bool checked)
{

}

void SetupOutputDialog::writeOutputFile()
{
    //first copy the values from the table to the stringlists
    if (ui.calcSettingsTable->rowCount() != 0)
    {
        int line = settingEntries_+4+((linesPerEntry_+3)*ui.calcsList->currentRow());
        for (int i = 0; i < linesPerEntry_; i++)
        {
            outputValues.replace(line, ui.calcSettingsTable->item(i,1)->text());
            line++;
        }
    }

    QDir::setCurrent(workingDir_);

    QFile fileWrite(workingDir_+"temp.txt");

    if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open temporary file");
        msgBox.exec();
        return;
    }

    QTextStream streamWrite(&fileWrite);
    QString line;
    QString original;

    streamWrite << outputFileName_ << "." << outputSetupFileType_.toUpper() << "               Title of this file\n"
                << "fnameato    " << atoFileName_ << "               Name of .ato file\n";

    for (int i = 0; i < settingEntries_; i++)
    {
        line = ui.outputSettingsTable->item(i,0)->text()+ui.outputSettingsTable->item(i,1)->text()+"               "+ui.outputSettingsTable->item(i,2)->text()+"\n";
        original.append(line);
    }

    if (ui.calcsList->count() > 0)
    {
        original.append(outputKeywords.at(settingEntries_)+QString::number(ui.calcsList->count())+"               "+outputDescriptions.at(settingEntries_)+"\n");
        int entryLine = settingEntries_+1;
        for (int j = 0; j < ui.calcsList->count(); j++)
        {
            original.append("\n");
            entryLine++;
            original.append(outputKeywords.at(settingEntries_+2).split(" ", QString::SkipEmptyParts).at(0)+"   "+QString::number(j+1)+"\n");
            entryLine++;
            original.append("\n");
            entryLine++;
            for (int i = 0; i < linesPerEntry_; i++)
            {
                line = outputKeywords.at(entryLine)+outputValues.at(entryLine)+"               "+outputDescriptions.at(entryLine)+"\n";
                original.append(line);
                entryLine++;
            }
        }
    }
    original.append("q\n");
    fileWrite.resize(0);
    streamWrite << original;
    fileWrite.close();

    //rename temp file as box .ato file to copy over changes and delete temp file
    QFile::remove(workingDir_+outputFileName_+outputFileExt_);
    QFile::rename(workingDir_+"temp.txt", workingDir_+outputFileName_+outputFileExt_);

    accept();
}

void SetupOutputDialog::on_cancelButton_clicked(bool checked)
{

}

void SetupOutputDialog::on_addCalcButton_clicked(bool checked)
{
    int calcToAdd = ui.calcsList->count()+1;
    ui.calcsList->addItem(QString::number(calcToAdd));

    int line = settingEntries_+1;
    for (int i = 0; i < linesPerEntry_+3; i++)
    {
        outputKeywords.append(outputKeywords.at(line));
        outputValues.append(outputValues.at(line));
        outputDescriptions.append(outputDescriptions.at(line));
        line++;
    }

    ui.calcsList->setCurrentRow(calcToAdd-1);
}

void SetupOutputDialog::on_removeCalcButton_clicked(bool checked)
{
    if (ui.calcsList->count() < 2) return;  //don't allow first calc to be removed as this contains keywords and descriptions

    int calcToTake = ui.calcsList->currentRow();
    ui.calcsList->takeItem(calcToTake);

    int line = settingEntries_+1+((linesPerEntry_+3)*calcToTake);
    for (int i = 0; i < linesPerEntry_+3; i++)
    {
        //this only works if the calc removed is the last calc listed ***********************************************************************************************
        outputKeywords.removeAt(line);
        outputValues.removeAt(line);
        outputDescriptions.removeAt(line);
    }
}

void SetupOutputDialog::on_deleteOutputButton_clicked(bool checked)
{
    QFile::remove(workingDir_+outputFileName_+outputFileExt_);
    this->close();
}
