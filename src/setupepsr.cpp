#include <QtGui>
#include <QPushButton>
#include <QDebug>
#include <QProcess>
#include <QString>
#include <QFile>

#include "epsrproject.h"

void MainWindow::on_setupEPSRButton_clicked(bool checked)
{
    QDir::setCurrent(workingDir_);
    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);
    epsrInpFileName_ = workingDir_+atoBaseFileName+".EPSR.inp";

    //search folder to see if a .EPSR.inp of same name as box already exists
    if (!QFileInfo(epsrInpFileName_).exists())
    {
        QProcess processEpsrSetup;
        processEpsrSetup.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
        processEpsrSetup.start(epsrBinDir_+"upset.exe", QStringList() << workingDir_ << "upset" << "epsr" << atoBaseFileName);
#else
        processEpsrSetup.start(epsrBinDir_+"upset", QStringList() << workingDir_ << "upset" << "epsr" << atoBaseFileName);
#endif
        if (!processEpsrSetup.waitForStarted()) return;

        processEpsrSetup.write("fnameato\n");          // move to fnameato line
        QByteArray result = processEpsrSetup.readAll();
        qDebug(result);

        processEpsrSetup.write(qPrintable(atoBaseFileName+"\n\n"));
        result = processEpsrSetup.readAll();
        qDebug(result);

        processEpsrSetup.write(qPrintable(atoBaseFileName+"\n\n"));
        result = processEpsrSetup.readAll();
        qDebug(result);

        processEpsrSetup.write("ndata\n");          // move to ndata line
        result = processEpsrSetup.readAll();
        qDebug(result);

        int N_data = dataFileList.count();
        processEpsrSetup.write(qPrintable(QString::number(N_data)+"\n\n"));          // move to data section
        result = processEpsrSetup.readAll();
        qDebug(result);

        for (int file = 0; file < N_data; file++)
        {
            QString wholeDataFileName = dataFileList.at(file);
            QFileInfo fileInfo1(wholeDataFileName);
            QString dataFileName = fileInfo1.fileName();
            processEpsrSetup.write(qPrintable(dataFileName+"\n\n"));
            result = processEpsrSetup.readAll();
            qDebug(result);

            QString wholeWtsFileName = wtsFileList.at(file);
            QFileInfo fileInfo2(wholeWtsFileName);
            QString wtsFileName = fileInfo2.fileName();
            processEpsrSetup.write(qPrintable(wtsFileName+"\n\n"));            //get from column 3 in wts file table
            result = processEpsrSetup.readAll();
            qDebug(result);

    //        QString dataType = dataFileTypeList.at(dataFile);
            processEpsrSetup.write(qPrintable("5\n\n"));            //NEED TO CHANGE THIS FOR OTHER DATA FORMATS
            result = processEpsrSetup.readAll();
            qDebug(result);

            processEpsrSetup.write("\n\n\n\n");
            result = processEpsrSetup.readAll();
            qDebug(result);
        }

        processEpsrSetup.write("e\n");
        result = processEpsrSetup.readAll();
        qDebug(result);

        processEpsrSetup.write("\n");
        result = processEpsrSetup.readAll();
        qDebug(result);

        if (!processEpsrSetup.waitForFinished(60000)) return;

        printf("\nfinished making EPSR setup file %s\n", qPrintable(epsrInpFileName_));
    }

    readEPSRinpFile();
    updateInpFileTables();
    readEPSRpcofFile();
    updatePcofFileTables();
    ui.epsrInpFileName->setText(epsrInpFileName_);
    ui.dataFileBrowseButton->setDisabled(true);
    ui.removeDataFileButton->setDisabled(true);
    ui.checkAct->setEnabled(true);
    ui.runAct->setEnabled(true);
    ui.stopAct->setEnabled(true);
    ui.plotAct->setEnabled(true);

    ui.messagesLineEdit->setText("EPSR .inp file created");
}

void MainWindow::on_reloadEPSRinpButton_clicked(bool checked)
{
    readEPSRinpFile();
    updateInpFileTables();
    readEPSRpcofFile();
    updatePcofFileTables();
    ui.messagesLineEdit->setText("EPSR .inp and .pcof files reloaded");
}

void MainWindow::readEPSRinpFile()
{
    QFile file(epsrInpFileName_);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open EPSR.inp file");
        msgBox.exec();
        return;
    }

    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    inpDescriptions.clear();
    QStringList inp1stcolumns;
    inp1stcolumns.clear();
    inpKeywords.clear();
    inpValues.clear();
    inpEntries_ = 0;
    datafilesettings.clear();

    line = stream.readLine();
    for (int i = 0; i < 100; i++)
    {
        line = stream.readLine();
        dataLine = line.split("               ", QString::SkipEmptyParts);
        if (dataLine.count() == 0)
            break;
        inp1stcolumns.append(dataLine.at(0));
        inpDescriptions.append(dataLine.at(1));
        inpEntries_++;
    }

    int ndatasets = dataFileList.count();
    for (int dataset = 0; dataset < ndatasets; dataset++)
    {
        line = stream.readLine();
        line = stream.readLine();
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        datafilesettings.append(DataFileSettings());
        datafilesettings.last().datafile=dataLine.at(1);
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        datafilesettings.last().wtsfile=dataLine.at(1);
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        datafilesettings.last().nrtype=dataLine.at(1);
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        datafilesettings.last().rshmin=dataLine.at(1);
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        datafilesettings.last().szeros=dataLine.at(1);
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        datafilesettings.last().tweak=dataLine.at(1);
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        datafilesettings.last().efilereq=dataLine.at(1);
        line = stream.readLine();
    }
    file.close();

    const int N_InpEntries = inpDescriptions.count();

    QRegExp inpFilerx("^.{12}");
    QString inpKeyword;
    QString inpValue;

    for (int i = 0; i < N_InpEntries; i++)
    {
        inpKeyword = inp1stcolumns.at(i).split(" ", QString::SkipEmptyParts).at(0);
        inpKeywords.append(inpKeyword);
        inpValue = inp1stcolumns.at(i).split(inpFilerx, QString::SkipEmptyParts).at(0);
        inpValues.append(inpValue);
    }
}

void MainWindow::updateInpFileTables()
{
    int N_InpValues = inpDescriptions.count();
    int nDatasets = dataFileList.count();

    ui.inpSettingsTable->clear();
    ui.dataFileSettingsTable->clear();

    //EPSR.inp settings
    ui.inpSettingsTable->setColumnWidth(1,180);
    ui.inpSettingsTable->setColumnWidth(2,500);
    ui.inpSettingsTable->setRowCount(inpEntries_-1);       //this is up until the 1st data file
    ui.inpSettingsTable->horizontalHeader()->setVisible(true);
    QStringList inpheader;
    inpheader << "EPSR keyword" << "Value" << "Description";
    ui.inpSettingsTable->setHorizontalHeaderLabels(inpheader);
    ui.inpSettingsTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < inpEntries_-1; i++)
    {
        QTableWidgetItem *itemkeyword = new QTableWidgetItem(inpKeywords.at(i));
        itemkeyword->setFlags(itemkeyword->flags() & ~Qt::ItemIsEditable);
        ui.inpSettingsTable->setItem(i,0, itemkeyword);
        ui.inpSettingsTable->setItem(i,1, new QTableWidgetItem(inpValues.at(i)));
        QTableWidgetItem *itemdescrip = new QTableWidgetItem(inpDescriptions.at(i));
        itemdescrip->setFlags(itemdescrip->flags() & ~Qt::ItemIsEditable);
        ui.inpSettingsTable->setItem(i,2, itemdescrip);
    }
    ui.inpSettingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.inpSettingsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.inpSettingsTable->setCurrentCell(N_InpValues-1,0);

    //EPSR .inp data file settings
    ui.dataFileSettingsTable->setColumnWidth(1,120);
    ui.dataFileSettingsTable->setColumnWidth(2,300);
    ui.dataFileSettingsTable->setRowCount(nDatasets*7);       //just data files
    ui.inpSettingsTable->horizontalHeader()->setVisible(true);
    ui.dataFileSettingsTable->setHorizontalHeaderLabels(inpheader);
    ui.dataFileSettingsTable->verticalHeader()->setVisible(false);

    int row = 0;
    for (int n = 0; n < nDatasets; ++n)
    {
        ui.dataFileSettingsTable->setItem(row,0, new QTableWidgetItem("datafile"));
        ui.dataFileSettingsTable->setItem(row,1, new QTableWidgetItem(datafilesettings.at(n).datafile));
        ui.dataFileSettingsTable->setItem(row++,2, new QTableWidgetItem("Name of data file to be fit"));
        ui.dataFileSettingsTable->setItem(row,0, new QTableWidgetItem("wtsfile"));
        ui.dataFileSettingsTable->setItem(row,1, new QTableWidgetItem(datafilesettings.at(n).wtsfile));
        ui.dataFileSettingsTable->setItem(row++,2, new QTableWidgetItem("Name of weights file for this data set"));
        ui.dataFileSettingsTable->setItem(row,0, new QTableWidgetItem("nrtype"));
        ui.dataFileSettingsTable->setItem(row,1, new QTableWidgetItem(datafilesettings.at(n).nrtype));
        ui.dataFileSettingsTable->setItem(row++,2, new QTableWidgetItem("Data type - see User Manual for more details"));
        ui.dataFileSettingsTable->setItem(row,0, new QTableWidgetItem("rshmin"));
        ui.dataFileSettingsTable->setItem(row,1, new QTableWidgetItem(datafilesettings.at(n).rshmin));
        ui.dataFileSettingsTable->setItem(row++,2, new QTableWidgetItem("Minimum radius [A] - used for background subtraction"));
        ui.dataFileSettingsTable->setItem(row,0, new QTableWidgetItem("szeros"));
        ui.dataFileSettingsTable->setItem(row,1, new QTableWidgetItem(datafilesettings.at(n).szeros));
        ui.dataFileSettingsTable->setItem(row++,2, new QTableWidgetItem("Zero limit - 0 means use first data point for Q=0"));
        ui.dataFileSettingsTable->setItem(row,0, new QTableWidgetItem("tweak"));
        ui.dataFileSettingsTable->setItem(row,1, new QTableWidgetItem(datafilesettings.at(n).tweak));
        ui.dataFileSettingsTable->setItem(row++,2, new QTableWidgetItem("Scaling factor for this data set. [1.0]"));
        ui.dataFileSettingsTable->setItem(row,0, new QTableWidgetItem("efilereq"));
        ui.dataFileSettingsTable->setItem(row,1, new QTableWidgetItem(datafilesettings.at(n).efilereq));
        ui.dataFileSettingsTable->setItem(row++,2, new QTableWidgetItem("Requested energy amplitude for this data set [1.0]"));
        ui.dataFileSettingsTable->insertRow(row++);
    }

    ui.dataFileSettingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.dataFileSettingsTable->setSelectionMode(QAbstractItemView::SingleSelection);
//    ui.dataFileSettingsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); full read only table
    ui.dataFileSettingsTable->setCurrentCell(nDatasets*8-1,0);
}

bool MainWindow::readEPSRpcofFile()
{
    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);
    QString epsrpcofFileName = workingDir_+atoBaseFileName+".pcof";
    QFile file(epsrpcofFileName);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open EPSR.pcof file");
        msgBox.exec();
        return false;
    }

    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    pcofDescriptions.clear();
    QStringList pcof1stcolumns;
    pcof1stcolumns.clear();
    pcofKeywords.clear();
    pcofValues.clear();
    atomPairs.clear();
    minDistances.clear();

    do {
        line = stream.readLine();
        dataLine = line.split("               ", QString::SkipEmptyParts);
        if (dataLine.at(0) == "q") break;
        pcof1stcolumns.append(dataLine.at(0));
        pcofDescriptions.append(dataLine.at(1));
    } while (!line.isNull());

    const int N_pcofEntries = pcofDescriptions.count();

    QRegExp pcofFilerx("^.{12}");
    QString pcofKeyword;
    QString pcofValue;

    for (int j = 0; j < N_pcofEntries; j++)
    {
        pcofKeyword = pcof1stcolumns.at(j).split(" ", QString::SkipEmptyParts).at(0);
        pcofKeywords.append(pcofKeyword);
        pcofValue = pcof1stcolumns.at(j).split(pcofFilerx, QString::SkipEmptyParts).at(0);
        pcofValues.append(pcofValue);
    }

    //read in minimum distance restraints
    line = stream.readLine();
//    line = stream.readLine();

    for (int i = 0; i < nPartials; i++)
    {
    line = stream.readLine();
    atomPairs.append(line);
    line = stream.readLine();
    QString minDistance = line.split(" ", QString::SkipEmptyParts).at(0);
    minDistances.append(minDistance);
    line = stream.readLine();
//    printf("%s", qPrintable(atomPairs.at(i)));
//    printf("%s", qPrintable(minDistances.at(i)));
    }
    file.close();

    return true;
}

void MainWindow::updatePcofFileTables()
{
    int N_pcofValues = pcofDescriptions.count();

    //EPSR .pcof settings
    ui.pcofSettingsTable->setColumnWidth(2,500);
    ui.pcofSettingsTable->setRowCount(18);       //this is just the pcof settings
    ui.pcofSettingsTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < 18; i++)
    {
        QTableWidgetItem *itemkeyword = new QTableWidgetItem(pcofKeywords.at(i));
        itemkeyword->setFlags(itemkeyword->flags() & ~Qt::ItemIsEditable);
        ui.pcofSettingsTable->setItem(i,0, itemkeyword);
        ui.pcofSettingsTable->setItem(i,1, new QTableWidgetItem(pcofValues.at(i)));
        QTableWidgetItem *itemdescrip = new QTableWidgetItem(pcofDescriptions.at(i));
        itemdescrip->setFlags(itemdescrip->flags() & ~Qt::ItemIsEditable);
        ui.pcofSettingsTable->setItem(i,2, itemdescrip);
    }
    ui.pcofSettingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.pcofSettingsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.pcofSettingsTable->setCurrentCell(N_pcofValues-1,0);

    //EPSR minimum intermolecular atom pair distances
    ui.minDistanceTable->setColumnWidth(1,180);
    ui.minDistanceTable->setRowCount(nPartials);       //this is just the pcof settings
    ui.minDistanceTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < nPartials; i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(atomPairs.at(i));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui.minDistanceTable->setItem(i,0, item);
        ui.minDistanceTable->setItem(i,1, new QTableWidgetItem(minDistances.at(i)));
    }
    ui.minDistanceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.minDistanceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.minDistanceTable->setCurrentCell(nPartials-1,0);
}

void MainWindow::updateInpFile()
{
//    ui.statusBar->show();

    QDir::setCurrent(workingDir_);

    QFile fileWrite(workingDir_+"tempinp.txt");

    if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open temporary inp file");
        msgBox.exec();
        return;
    }

    QTextStream streamWrite(&fileWrite);

    inpKeywords.clear();
    inpValues.clear();
    inpDescriptions.clear();
    datafilesettings.clear();

    int nDatasets = dataFileList.count();
    QString nDatasetsstr;
    QString nstr;

    //take values from tables in inp tab and put into QStringLists
    for (int i = 0; i < ui.inpSettingsTable->rowCount() ; ++i)
    {
        inpKeywords.append(ui.inpSettingsTable->item(i,0)->text());
        inpValues.append(ui.inpSettingsTable->item(i,1)->text());
        inpDescriptions.append(ui.inpSettingsTable->item(i,2)->text());
    }
    inpKeywords.append("ndata");
    inpValues.append(nDatasetsstr.number(nDatasets));
    inpDescriptions.append("Number of data files to be fit by EPSR");

    int row = 0;

    for (int j = 0; j < nDatasets; ++j)
    {
        datafilesettings.append(DataFileSettings());
        datafilesettings.last().datafile=ui.dataFileSettingsTable->item(row++,1)->text();
        datafilesettings.last().wtsfile=ui.dataFileSettingsTable->item(row++,1)->text();
        datafilesettings.last().nrtype=ui.dataFileSettingsTable->item(row++,1)->text();
        datafilesettings.last().rshmin=ui.dataFileSettingsTable->item(row++,1)->text();
        datafilesettings.last().szeros=ui.dataFileSettingsTable->item(row++,1)->text();
        datafilesettings.last().tweak=ui.dataFileSettingsTable->item(row++,1)->text();
        datafilesettings.last().efilereq=ui.dataFileSettingsTable->item(row++,1)->text();
        row++;
    }

    fileWrite.resize(0);

    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

    streamWrite << atoBaseFileName+".EPSR               Title of this file\n";
    for (int i = 0; i < inpKeywords.count(); i++)
    {
        streamWrite << inpKeywords.at(i)+"  "+inpValues.at(i)+"               "+inpDescriptions.at(i)+"\n";
    }
    int n;
    for (n = 0; n < datafilesettings.count(); n++)
    {
        streamWrite << "\n";
        streamWrite << "data   "+nstr.number(n+1)+"\n";
        streamWrite << "\n";
        streamWrite << "datafile    "+datafilesettings.at(n).datafile+"               Name of data file to be fit\n";
        streamWrite << "wtsfile    "+datafilesettings.at(n).wtsfile+"               Name of weights file for this data set\n";
        streamWrite << "nrtype    "+datafilesettings.at(n).nrtype+"               Data type - see User Manual for more details\n";
        streamWrite << "rshmin    "+datafilesettings.at(n).rshmin+"               Minimum radius [A] - used for background subtraction\n";
        streamWrite << "szeros    "+datafilesettings.at(n).szeros+"               Zero limit - 0 means use first data point for Q=0\n";
        streamWrite << "tweak    "+datafilesettings.at(n).tweak+"               Scaling factor for this data set. [1.0]\n";
        streamWrite << "efilereq    "+datafilesettings.at(n).efilereq+"               Requested energy amplitude for this data set [1.0]\n";
    }
    streamWrite << "q\n";
    fileWrite.close();

    QFile fileRead(epsrInpFileName_);
    fileRead.remove();
    fileWrite.rename(epsrInpFileName_);

    //now format the file correctly by opening in setup epsr and then exiting and saving
    QProcess processEpsrSetup;
    processEpsrSetup.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processEpsrSetup.start(epsrBinDir_+"upset.exe", QStringList() << workingDir_ << "upset" << "epsr" << atoBaseFileName);
#else
    processEpsrSetup.start(epsrBinDir_+"upset", QStringList() << workingDir_ << "upset" << "epsr" << atoBaseFileName);
#endif
    if (!processEpsrSetup.waitForStarted()) return;

    processEpsrSetup.write("e\n");
    QByteArray result = processEpsrSetup.readAll();
    qDebug(result);

    processEpsrSetup.write("\n");
    result = processEpsrSetup.readAll();
    qDebug(result);

    processEpsrSetup.write("y\n");
    result = processEpsrSetup.readAll();
    qDebug(result);

    if (!processEpsrSetup.waitForFinished(60000)) return;

//    printf("\nfinished updating EPSR setup file %s\n", qPrintable(epsrInpFileName_));

    updateInpFileTables();
}

void MainWindow::updatePcofFile()
{
    QDir::setCurrent(workingDir_);

    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);
    QString epsrpcofFileName = workingDir_+atoBaseFileName+".pcof";

    QFile fileRead(epsrpcofFileName);
    QFile fileWrite(workingDir_+"temppcof.txt");

    if(!fileRead.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open EPSR.pcof file");
        msgBox.exec();
        return;
    }

    if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open temporary pcof file");
        msgBox.exec();
        return;
    }

    QTextStream streamRead(&fileRead);
    QString lineRead;
    QStringList dataLineRead;
    dataLineRead.clear();
    QString original;
    QTextStream streamWrite(&fileWrite);
    QStringList dataLineWrite;
    dataLineWrite.clear();

    pcofValues.clear();
    minDistances.clear();

    QString nPartialsstr;

    //take values from tables in inp tab and put into QStringLists
    for (int i = 0; i < ui.pcofSettingsTable->rowCount() ; ++i)
    {
        pcofValues.append(ui.pcofSettingsTable->item(i,1)->text());
    }

    for (int j = 0; j < nPartials; ++j)
    {
        minDistances.append(ui.minDistanceTable->item(j,1)->text());
    }

//    printf("%s\n", qPrintable(datafilesettings.at(0).datafile));

//    fileWrite.resize(0);
    for (int k = 0; k < pcofKeywords.count(); k++)
    {
        streamWrite << pcofKeywords.at(k)+"  "+pcofValues.at(k)+"               "+pcofDescriptions.at(k)+"\n";
    }
    streamWrite << "q\n";
    streamWrite << "          "+nPartialsstr.number(nPartials)+"\n";

    do {
        lineRead = streamRead.readLine();
        dataLineRead = lineRead.split(" ", QString::SkipEmptyParts);
        if (dataLineRead.at(0) == "q") break;
    } while (!lineRead.isNull());

    lineRead = streamRead.readLine(); //this is the number of partials line

    for (int n = 0; n < nPartials; n++)
    {
        lineRead = streamRead.readLine();
        original.append(lineRead+"\n");    //this writes the atom pairs (can also use atomPairs.at(n))
        lineRead = streamRead.readLine();
        original.append("   "+minDistances.at(n)+"      0.00000000\n"); //THE SECOND NUMBER IS THE MAX DISTANCE - if Alan implements the use of this then will need to edit this line of code to accomodate
        lineRead = streamRead.readLine();
        original.append(lineRead+"\n");
    }
    fileWrite.resize(0);
    streamWrite << original;
    fileWrite.close();
    fileRead.close();

    fileRead.remove();
    fileWrite.rename(epsrpcofFileName);

    //now format the file correctly by opening in setup epsr and then exiting and saving
    QProcess processEpsrSetup;
    processEpsrSetup.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processEpsrSetup.start(epsrBinDir_+"upset.exe", QStringList() << workingDir_ << "upset" << "epsr" << atoBaseFileName);
#else
    processEpsrSetup.start(epsrBinDir_+"upset", QStringList() << workingDir_ << "upset" << "epsr" << atoBaseFileName);
#endif
    if (!processEpsrSetup.waitForStarted()) return;

    processEpsrSetup.write("e\n");
    QByteArray result = processEpsrSetup.readAll();
    qDebug(result);

    processEpsrSetup.write("\n");
    result = processEpsrSetup.readAll();
    qDebug(result);

    processEpsrSetup.write("y\n");
    result = processEpsrSetup.readAll();
    qDebug(result);

    if (!processEpsrSetup.waitForFinished(60000)) return;

//    printf("\nfinished updating EPSR setup file %s\n", qPrintable(epsrInpFileName_));

    updatePcofFileTables();
}

void MainWindow::on_updateInpPcofFilesButton_clicked(bool checked)
{
    updateInpFile();
    updatePcofFile();
}
