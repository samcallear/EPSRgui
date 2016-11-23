#include <QtGui>
#include <QPushButton>
#include <QDebug>
#include <QProcess>
#include <QString>
#include <QFile>

#include "epsrproject.h"
#include "wtscomponent.h"
#include "datafilesettings.h"

void MainWindow::on_dataFileBrowseButton_clicked(bool checked)
{
    QString newDataFileName;
    if (ui.neutronDataRB->isChecked())
    {
        QString filters(".mint01 files (*.mint01);;All files (*.*)");
        QString defaultFilter(".mint01 files (*.mint01)");
        newDataFileName = QFileDialog::getOpenFileName(this, "Choose dataset", workingDir_, filters, &defaultFilter);
    }
    else
    if (ui.xrayDataRB->isChecked())
    {
        QString filters(".int01 files (*.int01);;All files (*.*)");
        QString defaultFilter(".int01 files (*.int01)");
        newDataFileName = QFileDialog::getOpenFileName(this, "Choose dataset", workingDir_, filters, &defaultFilter);
    }
    if (!newDataFileName.isEmpty())
    {
        QString dataFilePath = QFileInfo(newDataFileName).path()+"/";
        dataFilePath = QDir::toNativeSeparators(dataFilePath);
        QFileInfo dataFileInfo(newDataFileName);
        QString dataFileName;
        dataFileName = dataFileInfo.fileName();

        //check if the selected file is already listed as a dataset in the project
        for (int i = 0; i < ui.dataFileTable->rowCount(); i++)
        {
            if (ui.dataFileTable->item(i,0)->text() == dataFileName)
            {
                QMessageBox msgBox;
                msgBox.setText("This data file is already listed in the project.");
                msgBox.exec();
                return;
            }
        }

        //if necessary, copy to workingDir_ (question if a file of the same name is already present there)
        if (dataFilePath != workingDir_)
        {
            if (QFile::exists(workingDir_+dataFileName) == true)
            {
                QMessageBox::StandardButton msgBox;
                msgBox  = QMessageBox::question(this, "Warning", "This will overwrite the data file already present in the project folder with the same name.\nProceed?", QMessageBox::Ok|QMessageBox::Cancel);
                if (msgBox == QMessageBox::Cancel)
                {
                    return;
                }
                else
                {
                    QFile::copy(newDataFileName, workingDir_+dataFileName);
                }
            }
            else
            {
                QFile::copy(newDataFileName, workingDir_+dataFileName);
            }
        }

        wtsBaseFileName_ = dataFileName.split(".",QString::SkipEmptyParts).at(0);

        ui.dataFileLineEdit->setText(newDataFileName);

        dataFileList.append(dataFileName);
        dataFileTypeList.append("5");

        dataFileName_ = dataFileName;
        if (ui.neutronDataRB->isChecked())
        {
            normalisationList.append("0");
            makeNwtsSetup();
            wtsFileList.append("  ");
            readNwtsSetup();
            refreshDataFileTable();
        }
        else
        if (ui.xrayDataRB->isChecked())
        {
            normalisationList.append("2");
            makeXwtsSetup();
            wtsFileList.append("  ");
            readXwtsSetup();
            refreshDataFileTable();
        }
    }
    ui.makeWtsButton->setEnabled(true);
    ui.setupEPSRButton->setEnabled(true);

    //save .pro file
    save();

    ui.messagesLineEdit->setText("New data file added");
}

bool MainWindow::makeNwtsSetup()
{
    QDir::setCurrent(workingDir_);
    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

    //create wtsBaseFileName_.NWTS.dat if it doesn't already exist
    if (QFile::exists(workingDir_+wtsBaseFileName_+".NWTS.dat") == 0)
    {
#ifdef _WIN32
        processEPSR_.start(epsrBinDir_+"upset.exe", QStringList() << workingDir_ << "upset" << "nwts" << wtsBaseFileName_);
#else
        processEPSR_.start(epsrBinDir_+"upset", QStringList() << workingDir_ << "upset" << "nwts" << wtsBaseFileName_);
#endif
        if (!processEPSR_.waitForStarted()) return false;

        processEPSR_.write("\n");          // move to fnameato line
        processEPSR_.write(qPrintable(atoBaseFileName+"\n"));
        processEPSR_.write("\n");
        processEPSR_.write("e\n");
        processEPSR_.write("\n");

        if (!processEPSR_.waitForFinished()) return false;
        messageText_ += "\nfinished making wts setup file\n";
    }

    //read values in NWTS.dat file and write to table and combobox
    messagesDialog.refreshMessages();
    ui.messagesLineEdit->setText("NWTS setup file created");
    return true;
}

bool MainWindow::makeXwtsSetup()
{
    // if wtsBaseFileName_.xwts.dat already exists don't do the following, but do import it into the table
    QDir::setCurrent(workingDir_);
    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);
    if (QFile::exists(workingDir_+wtsBaseFileName_+".XWTS.dat") == 0)
    {
#ifdef _WIN32
        processEPSR_.start(epsrBinDir_+"upset.exe", QStringList() << workingDir_ << "upset" << "xwts" << wtsBaseFileName_);
#else
        processEPSR_.start(epsrBinDir_+"upset", QStringList() << workingDir_ << "upset" << "xwts" << wtsBaseFileName_);
#endif
        if (!processEPSR_.waitForStarted()) return false;

        processEPSR_.write("\n");          // move to fnameato line
        processEPSR_.write(qPrintable(atoBaseFileName+"\n"));
        processEPSR_.write("\n");
        processEPSR_.write("e\n");
        processEPSR_.write("\n");

        if (!processEPSR_.waitForFinished()) return false;
        messageText_ += "\nfinished making wts setup file\n";
    }
    messagesDialog.refreshMessages();
    ui.messagesLineEdit->setText("XWTS setup file created");
    return true;
}

bool MainWindow::readNwtsSetup()
{
    QString wtsFileName = workingDir_+wtsBaseFileName_+".NWTS.dat";
    QFile file(wtsFileName);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open wts setup file");
        msgBox.exec();
        return false;
    }
    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    wtscomponents.clear();
    int normdataType = 0;

    do
    {
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        if (dataLine.count() == 0) continue;
            if (dataLine.at(0) == "normtot")
            {
                normdataType = dataLine.at(1).toInt();
            }
            if (dataLine.at(0) == "component")
            {
                wtscomponents.append(WtsComponent());
            }
            if (dataLine.at(0) == "atom")
            {
                wtscomponents.last().atom=dataLine.at(1);
            }
            if (dataLine.at(0) == "atsymbol")
            {
                wtscomponents.last().atsymbol=dataLine.at(1);
            }
            if (dataLine.at(0) == "iexchange")
            {
                wtscomponents.last().iexchange=dataLine.at(1);
            }
            if (dataLine.at(0) == "abundances")
            {
                  if (dataLine.at(3) != "List")
                  {
                        wtscomponents.last().isotope1=dataLine.at(1);
                        wtscomponents.last().abundance1=dataLine.at(2);
                        wtscomponents.last().isotope2=dataLine.at(3);
                        wtscomponents.last().abundance2=dataLine.at(4);
                  }
                  else
                  {
                      wtscomponents.last().isotope1=dataLine.at(1);
                      wtscomponents.last().abundance1=dataLine.at(2);
                  }
            }
    } while (!stream.atEnd());
    file.close();

    const int N_components = wtscomponents.count();

    //show data in Tables and combobox
    ui.atomWtsTable->setColumnCount(6);
    ui.atomWtsTable->setRowCount(N_components);
    QStringList wtsheader;
    wtsheader << "Atom Label" << "Exchangeable?" << "Isotope" << "Abundance" << "Isotope" << "Abundance";
    ui.atomWtsTable->setHorizontalHeaderLabels(wtsheader);
    ui.atomWtsTable->verticalHeader()->setVisible(false);
    ui.atomWtsTable->horizontalHeader()->setVisible(true);
    for (int i = 0; i < N_components; ++i)
    {
        QTableWidgetItem *item = new QTableWidgetItem(wtscomponents.at(i).atom);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui.atomWtsTable->setItem(i,0, item);
        ui.atomWtsTable->setItem(i,1, new QTableWidgetItem(wtscomponents.at(i).iexchange));
        ui.atomWtsTable->setItem(i,2, new QTableWidgetItem(wtscomponents.at(i).isotope1));
        ui.atomWtsTable->setItem(i,3, new QTableWidgetItem(wtscomponents.at(i).abundance1));
        ui.atomWtsTable->setItem(i,4, new QTableWidgetItem(wtscomponents.at(i).isotope2));
        ui.atomWtsTable->setItem(i,5, new QTableWidgetItem(wtscomponents.at(i).abundance2));
    }
    ui.atomWtsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.atomWtsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    //and on comboBox
    ui.normalisationComboBox->setCurrentIndex(normdataType);

    //update normalisationList
    QString normdataTypeStr = QString::number(normdataType);
    int selectedRow = ui.dataFileTable->currentRow();
    if (selectedRow != -1)
    {
        normalisationList.at(selectedRow) == normdataTypeStr;
    }

    return true;
}

bool MainWindow::readXwtsSetup()
{
    QString wtsFileName = workingDir_+wtsBaseFileName_+".XWTS.dat";
    QFile file(wtsFileName);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open wts setup file");
        msgBox.exec();
        return false;
    }
    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    wtscomponents.clear();
    int normdataType = 2;

    do
    {
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        if (dataLine.count() != 0)
        {
            if (dataLine.at(0) == "normtot")
            {
                normdataType = dataLine.at(1).toInt();
            }
            if (dataLine.at(0) == "component")
            {
                wtscomponents.append(WtsComponent());
            }
            if (dataLine.at(0) == "atom")
            {
                wtscomponents.last().atom=dataLine.at(1);
            }
            if (dataLine.at(0) == "atsymbol")
            {
                wtscomponents.last().atsymbol=dataLine.at(1);
            }
        }
    } while (!stream.atEnd());
    file.close();

    const int N_components = wtscomponents.count();

    //show data in Tables
    ui.atomWtsTable->setColumnCount(1);
    ui.atomWtsTable->setRowCount(N_components);
    QStringList wtsheader;
    wtsheader << "Atom Label";
    ui.atomWtsTable->setHorizontalHeaderLabels(wtsheader);
    ui.atomWtsTable->verticalHeader()->setVisible(false);
    ui.atomWtsTable->horizontalHeader()->setVisible(true);
    for (int i = 0; i < N_components; ++i)
    {
        QTableWidgetItem *item = new QTableWidgetItem(wtscomponents.at(i).atom);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui.atomWtsTable->setItem(i,0, item);
    }
    ui.atomWtsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    //and on comboBox
    ui.normalisationComboBox->setCurrentIndex(normdataType);

    //update normalisationList
    QString normdataTypeStr = QString::number(normdataType);
    int selectedRow = ui.dataFileTable->currentRow();
    if (selectedRow != -1)
    {
        normalisationList.at(selectedRow) == normdataTypeStr;
    }
    return true;
}

void MainWindow::on_makeWtsButton_clicked(bool checked)
{
    if (dataFileList.count() == 0)
    {
        return;
    }

    //update dataFileTypeList to contain what is in dataFileTable
    dataFileTypeList.clear();
    for (int i = 0; i < ui.dataFileTable->rowCount(); i++)
    {
        dataFileTypeList.append(ui.dataFileTable->item(i,1)->text());
    }

    //make weights files
    if (QFile::exists(workingDir_+wtsBaseFileName_+".NWTS.dat"))
    {
        makeNwts();
//        ui.messagesLineEdit->setText("NWTS wts file created");
    }
    if (QFile::exists(workingDir_+wtsBaseFileName_+".XWTS.dat"))
    {
        makeXwts();
//        ui.messagesLineEdit->setText("XWTS wts file created");
    }

    //save .pro file
    save();
}

void MainWindow::makeNwts()
{
    for (int i = 0; i < ui.atomWtsTable->rowCount(); i++)
    {
        if (ui.atomWtsTable->item(i,1)->text().isEmpty() || ui.atomWtsTable->item(i,2)->text().isEmpty() || ui.atomWtsTable->item(i,3)->text().isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("One of the parameters defining the scattering weights is missing");
            msgBox.exec();
            return;
        }
        if (!ui.atomWtsTable->item(i,4)->text().isEmpty() && ui.atomWtsTable->item(i,5)->text().isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("One of the parameters defining the isotope concentration is missing");
            msgBox.exec();
            return;
        }
        if (ui.atomWtsTable->item(i,4)->text().isEmpty() && !ui.atomWtsTable->item(i,5)->text().isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("One of the parameters defining the isotope concentration is missing");
            msgBox.exec();
            return;
        }
    }

    QDir::setCurrent(workingDir_);

    QFile fileRead(workingDir_+wtsBaseFileName_+".NWTS.dat");
    QFile fileWrite(workingDir_+"tempwts.txt");

    if(!fileRead.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .NWTS.dat file");
        msgBox.exec();
        return;
    }
    if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open temporary wts file");
        msgBox.exec();
        return;
    }

    //read normalisation from combo box and write to table
    int dataNormType = ui.normalisationComboBox->currentIndex();
    QString dataNormTypeStr = QString::number(dataNormType);
    int row = ui.dataFileTable->currentRow();

    ui.dataFileTable->item(row,2)->setText(dataNormTypeStr);

    //make normalisationList consistent with dataFileTable normalisation column
    normalisationList.clear();
    for (int i = 0; i < dataFileList.count(); i++)
    {
        normalisationList.append(ui.dataFileTable->item(i,2)->text());
    }

    QString istr;
    const int N_components = wtscomponents.count();

    wtscomponents.clear();

    for (int i = 0; i < N_components; ++i)
    {
        wtscomponents.append(WtsComponent());
        wtscomponents.last().atom=ui.atomWtsTable->item(i,0)->text();
        wtscomponents.last().iexchange=ui.atomWtsTable->item(i,1)->text();
        wtscomponents.last().isotope1=ui.atomWtsTable->item(i,2)->text();
        wtscomponents.last().abundance1=ui.atomWtsTable->item(i,3)->text();
        wtscomponents.last().isotope2=ui.atomWtsTable->item(i,4)->text();
        wtscomponents.last().abundance2=ui.atomWtsTable->item(i,5)->text();
     }

    QTextStream streamRead(&fileRead);
    QTextStream streamWrite(&fileWrite);
    QString line;
    QStringList dataLine;
    dataLine.clear();
    QString original;

    for (int N_lines = 0; N_lines < 5; N_lines++)
    {
        line = streamRead.readLine();
        original.append(line+"\n");
    }
    line = streamRead.readLine();
    original.remove(line+"\n");
    original.append("normtot     "+dataNormTypeStr+"               Normalise totals to (0 = nothing, 1 = <b>^2, 2 = <b^2> \n");
    for (int N_lines = 0; N_lines < 3; N_lines++)
    {
        line = streamRead.readLine();
        original.append(line+"\n");
    }
    fileWrite.resize(0);
    streamWrite << original;
    for (int i = 0; i < N_components; i++)
    {
        streamWrite << "\ncomponent   "+istr.number(i+1)+"\n\n";
        streamWrite << "atom        "+wtscomponents.at(i).atom+"               Atom label - set from .ato file\n";
        streamWrite << "atsymbol    "+wtscomponents.at(i).atsymbol+"               Atom symbol - set from .ato file\n";
        streamWrite << "iexchange   "+wtscomponents.at(i).iexchange+"               1 if this atom exchanges with other atoms, 0 otherwise\n";
        streamWrite << "abundances  "+wtscomponents.at(i).isotope1+" "+wtscomponents.at(i).abundance1+" "+wtscomponents.at(i).isotope2+" "+wtscomponents.at(i).abundance2+"               List of mass numbers and abundances. 0 for natural isotope\n";
        streamWrite << "isubs       0               This atom isotope substituted? (1 = yes, 0 = no)\n";
        streamWrite << "abund2      0 1               List of mass numbers and abundances for second sample\n";
    }
    streamWrite << "q\n";

    fileWrite.close();
    fileRead.close();

    fileRead.remove();
    fileWrite.rename(workingDir_+wtsBaseFileName_+".NWTS.dat");

#ifdef _WIN32
    processEPSR_.start(epsrBinDir_+"nwts.exe", QStringList() << workingDir_ << "nwts" << wtsBaseFileName_);
#else
    processEPSR_.start(epsrBinDir_+"nwts", QStringList() << workingDir_ << "nwts" << wtsBaseFileName_);
#endif
    if (!processEPSR_.waitForStarted()) return;
    if (!processEPSR_.waitForFinished()) return;

    //check file was created (need to check if modification actually works too***********************)
    QFile wtsfile(workingDir_+wtsBaseFileName_+".NWTStot.wts");
    if (!wtsfile.exists())
    {
        QMessageBox msgBox;
        msgBox.setText("Could not make wts file");
        msgBox.exec();
        return;
    }

    wtsFileList.replace(row, wtsBaseFileName_+".NWTStot.wts");
    messageText_ += "\nfinished making wts file\n";
    messagesDialog.refreshMessages();
    ui.messagesLineEdit->setText("finished making wts file");
    refreshDataFileTable();
}

void MainWindow::makeXwts()
{
    QDir::setCurrent(workingDir_);

    QFile fileRead(workingDir_+wtsBaseFileName_+".XWTS.dat");
    QFile fileWrite(workingDir_+"tempxwts.txt");

    if(!fileRead.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .XWTS.dat file");
        msgBox.exec();
        return;
    }
    if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open temporary wts file");
        msgBox.exec();
        return;
    }

    //read normalisation from combo box and write to table
    int dataNormType = ui.normalisationComboBox->currentIndex();
    QString dataNormTypeStr = QString::number(dataNormType);
    int row = ui.dataFileTable->currentRow();

    ui.dataFileTable->item(row,2)->setText(dataNormTypeStr);

    //make normalisationList consistent with dataFileTable normalisation column
    normalisationList.clear();
    for (int i = 0; i < dataFileList.count(); i++)
    {
        normalisationList.append(ui.dataFileTable->item(i,2)->text());
    }

    QString istr;
    const int N_components = wtscomponents.count();

//    wtscomponents.clear();                  IF MAKE ANYTHING EDITABLE IN XWTS THEN NEED TO IMPLEMENT THIS
//
//    for (int i = 0; i < N_components; ++i)
//    {
//    wtscomponents.append(WtsComponent());
//    wtscomponents.last().atom=ui.atomWtsTable->item(i,0)->text();
//     }


    QTextStream streamRead(&fileRead);
    QTextStream streamWrite(&fileWrite);
    QString line;
    QStringList dataLine;
    dataLine.clear();
    QString original;

    for (int N_lines = 0; N_lines < 3; N_lines++)
    {
        line = streamRead.readLine();
        original.append(line+"\n");
    }
    line = streamRead.readLine();
    original.remove(line+"\n");
    original.append("normtot     "+dataNormTypeStr+"               Normalise totals to (0 = nothing, 1 = <b>^2, 2 = <b^2> \n");
    for (int N_lines = 0; N_lines < 3; N_lines++)
    {
        line = streamRead.readLine();
        original.append(line+"\n");
    }
    fileWrite.resize(0);
    streamWrite << original;
    for (int i = 0; i < N_components-1; i++)
    {
        streamWrite << "\ncomponent   "+istr.number(i+1)+"\n\n";
        streamWrite << "atom        "+wtscomponents.at(i).atom+"               Atom label - set from .ato file\n";
        streamWrite << "atsymbol    "+wtscomponents.at(i).atsymbol+"               Atom symbol - set from .ato file\n";
        streamWrite << "maffq       0               Modified atomic form factors alpha values (0";
        streamWrite << "maffdelta   0               Modified atomic form factors delta value (0)\n\n";
    }
    streamWrite << "q\n";

    fileWrite.close();
    fileRead.close();

    fileRead.remove();
    fileWrite.rename(workingDir_+wtsBaseFileName_+".XWTS.dat");

#ifdef _WIN32
    processEPSR_.start(epsrBinDir_+"xwts.exe", QStringList() << workingDir_ << "xwts" << wtsBaseFileName_);
#else
    processEPSR_.start(epsrBinDir_+"xwts", QStringList() << workingDir_ << "xwts" << wtsBaseFileName_);
#endif
    if (!processEPSR_.waitForStarted()) return;
    if (!processEPSR_.waitForFinished()) return;

    //check file was created (need to check if modification actually works too***********************)
    QFile wtsfile(workingDir_+wtsBaseFileName_+".XWTS.wts");
    if (!wtsfile.exists())
    {
        QMessageBox msgBox;
        msgBox.setText("Could not make wts file");
        msgBox.exec();
        return;
    }

    wtsFileList.replace(row, wtsBaseFileName_+".XWTS.wts");
    messageText_ += "\nfinished making wts file\n";
    messagesDialog.refreshMessages();
    ui.messagesLineEdit->setText("finished making wts file");
    refreshDataFileTable();
}

void MainWindow::refreshDataFileTable()
{
    ui.dataFileTable->clearContents();
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
}

void MainWindow::on_dataFileTable_itemSelectionChanged()
{
    int row = ui.dataFileTable->currentRow();
    if (row >= 0)
    {
        setSelectedDataFile();
    }
}

void MainWindow::setSelectedDataFile()
{
    int row = ui.dataFileTable->currentRow();
    QString dataFileName = dataFileList.at(row);
    QString wtsFileName = wtsFileList.at(row);
    if (wtsFileName.contains(" "))
    {
        wtsFileName = dataFileName;
    }
    QFileInfo fileInfo(wtsFileName);
    QString justFileName = fileInfo.fileName();
    wtsBaseFileName_ = justFileName.split(".",QString::SkipEmptyParts).at(0);
    dataFileName_ = dataFileName;

    if (QFile::exists(workingDir_+wtsBaseFileName_+".NWTS.dat"))
    {
        readNwtsSetup();
    }
    else
    if (QFile::exists(workingDir_+wtsBaseFileName_+".XWTS.dat"))
    {
        readXwtsSetup();
    }
}

void MainWindow::on_removeDataFileButton_clicked(bool checked)
{
    if (!epsrInpFileName_.isEmpty())
    {
        QMessageBox::StandardButton msgBox;
        msgBox =QMessageBox::question(this, "Warning", "An EPSR.inp file already exists for this project.\n"
                                                       " Removing this data file will mean the EPSR .inp file will need to be created again.\n"
                                                       " Press Ok to proceed.", QMessageBox::Ok|QMessageBox::Cancel);
        if (msgBox == QMessageBox::Cancel)
        {
            return;
        }
    }
    int row = ui.dataFileTable->currentRow();
    if (dataFileList.count() > 1)
    {
        dataFileList.takeAt(row);
        dataFileTypeList.takeAt(row);
        wtsFileList.takeAt(row);
        normalisationList.takeAt(row);
        setSelectedDataFile();
        refreshDataFileTable();
    }
    else
    {
        dataFileList.clear();
        dataFileTypeList.clear();
        wtsFileList.clear();
        normalisationList.clear();
        ui.normalisationComboBox->setCurrentIndex(0);
        ui.atomWtsTable->clearContents();
        ui.atomWtsTable->setRowCount(0);
        ui.dataFileTable->removeRow(0);
        ui.makeWtsButton->setEnabled(false);
        ui.setupEPSRButton->setEnabled(false);
    }

    //save .pro file
    save();

    ui.messagesLineEdit->setText("Data file removed");
}
