#include <QtGui>
#include <QPushButton>
#include <QDebug>
#include <QProcess>
#include <QString>
#include <QFile>
#include <QVector>

#include "epsrproject.h"
#include "molfiles.h"
#include "moloptionsdialog.h"
#include "boxcompositiondialog.h"
#include "addatodialog.h"

void MainWindow::on_mixatoButton_clicked(bool checked)
{
    if (ui.numberDensityLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Atomic number density is blank"),
            tr("Input an atomic number density for the simulation box."));
    }
    else
    {
        QString atoFileBaseName = projectName_+"box";
        atoFileName_ = atoFileBaseName+".ato";

        //make a list of all the .ato files in the working directory in alphabetical order and make
        //a list of the indexes of each of the .ato files want to mix
        QDir::setCurrent(workingDir_);

        QDir dir;
        QStringList atoFilter;
        atoFilter << "*.ato";
        QStringList atoFiles = dir.entryList(atoFilter, QDir::Files, QDir::Name|QDir::IgnoreCase);
        QStringList atoFileIndexes;
        QStringList numberOfMolecules;
        for (int i = 0; i < nMolFiles; i++)
        {
            QString atoFileName = ui.atoFileTable->item(i,0)->text();
            QString atoFileIndex = QString::number(atoFiles.indexOf(atoFileName));
            if (atoFileIndex == "-1")
            {
                QMessageBox msgBox;
                msgBox.setText("One of the .ato files could not be found in the working directory.\n Check that all the .ato files listed are present in the working directory and have the same case filename as the .mol files.");
                msgBox.exec();
                return;
            }
            atoFileIndexes.append(atoFileIndex);
            QString numberMol = ui.atoFileTable->item(i,2)->text();
            numberOfMolecules.append(numberMol);
        }

        int nIndex = atoFileIndexes.count();

//        processEPSR_.setProcessChannelMode(QProcess::ForwardedChannels);

        QString projDir = workingDir_;
        projDir = QDir::toNativeSeparators(projDir);
    #ifdef _WIN32
        processEPSR_.start(epsrBinDir_+"mixato.exe", QStringList() << projDir << "mixato");
    #else
        processEPSR_.start(epsrBinDir_+"mixato", QStringList() << workingDir_ << "mixato");
    #endif
        if (!processEPSR_.waitForStarted()) return;

        processEPSR_.write(qPrintable(QString::number(nIndex)+"\n"));


        for (int i = 0 ; i < nMolFiles; i++)
        {
            int newlines = atoFileIndexes.at(i).toInt();
            for (int nl = 0; nl < newlines; nl++)
            {
                processEPSR_.write("\n");

            }
            processEPSR_.write("y\n");


            int nMols = numberOfMolecules.at(i).toInt();
            processEPSR_.write(qPrintable(QString::number(nMols)+"\n"));

        }
            double numberDensity = ui.numberDensityLineEdit->text().toDouble();
            processEPSR_.write(qPrintable(QString::number(numberDensity)+"\n"));


            processEPSR_.write(qPrintable(atoFileBaseName+"\n"));


        if (!processEPSR_.waitForFinished(1800000)) return;

        messageText_ += "\nfinished writing "+atoFileName_+" file\n";
        messagesDialog.refreshMessages();
        ui.messagesLineEdit->setText("Finished writing box .ato file");

        ui.boxAtoLabel->setText(atoFileName_);
        readAtoFileAtomPairs();
        readAtoFileBoxDetails();
        ui.atoAtomList->clear();
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
}

void MainWindow::on_addatoButton_clicked(bool checked)
{
    //*****************
    //NOTE
    //There are limitations to what will be read during the process - see addato.f for details
    //the atomic number density is irrelevant as the size of the container determines the size of the final box.
    //the ecoredcore values at the bottom of the .mol files are important as they determine the atomic overlap during addato
    //tethering of molecules is also important prior to pressing addato.

    AddAtoDialog addAtoDialog(this);

    addAtoDialog.show();

    addAtoDialog.raise();
    addAtoDialog.activateWindow();

    atoaddDialog = addAtoDialog.exec();

    if (atoaddDialog == AddAtoDialog::Accepted)
    {
        QString container = addAtoDialog.getContainer();
        QStringList atoFilesToAdd = addAtoDialog.getAtoFiles();
        QStringList numberOfMolecules = addAtoDialog.getNumberMols();

        QString atoFileBaseName = projectName_+"box";
        atoFileName_ = atoFileBaseName+".ato";

        //make a list of all the .ato files in the working directory in alphabetical order and make
        //a list of the indexes of each of the .ato files want to mix
        QDir::setCurrent(workingDir_);

        QDir dir;
        QStringList atoFilter;
        atoFilter << "*.ato";
        QStringList atoFiles = dir.entryList(atoFilter, QDir::Files, QDir::Name|QDir::IgnoreCase);
        QStringList atoFileIndexes;
        atoFileIndexes.clear();

        int containerIndex = atoFiles.indexOf(container);
        if (containerIndex == -1)
        {
            QMessageBox msgBox;
            msgBox.setText("The container .ato file could not be found in the working directory.\n Check that all the .ato files listed are present in the working directory.");
            msgBox.exec();
            return;
        }

        for (int i = 0; i < atoFilesToAdd.count(); i++)
        {
            QString atoFileName = atoFilesToAdd.at(i);
            QString atoFileIndex = QString::number(atoFiles.indexOf(atoFileName));
            if (atoFileIndex == "-1")
            {
                QMessageBox msgBox;
                msgBox.setText("One of the .ato files could not be found in the working directory.\n Check that all the .ato files listed are present in the working directory.");
                msgBox.exec();
                return;
            }
            atoFileIndexes.append(atoFileIndex);
        }

        int nIndex = atoFilesToAdd.count(); //this is the number of files to be added to the container

//        processEPSR_.setProcessChannelMode(QProcess::ForwardedChannels);

        QString projDir = workingDir_;
        projDir = QDir::toNativeSeparators(projDir);
    #ifdef _WIN32
        processEPSR_.start(epsrBinDir_+"addato.exe", QStringList() << projDir << "addato");
    #else
        processEPSR_.start(epsrBinDir_+"addato", QStringList() << projDir << "addato");
    #endif
        if (!processEPSR_.waitForStarted()) return;

        processEPSR_.write(qPrintable(QString::number(nIndex)+"\n"));


        // press enter to get to each ato file listed in the table that will be added to the container
        for (int i = 0 ; i < atoFilesToAdd.count(); i++)
        {
            int newlines = atoFileIndexes.at(i).toInt();
            for (int nl = 0; nl < newlines; nl++)
            {
                processEPSR_.write("\n");

            }
            processEPSR_.write("y\n");


            processEPSR_.write(qPrintable(numberOfMolecules.at(i)+"\n"));

        }

        // press enter to get to the ato file that is the container
        for (int nl = 0; nl < containerIndex; nl++)
        {
            processEPSR_.write("\n");

        }
        processEPSR_.write("y\n");


        processEPSR_.write(qPrintable(atoFileBaseName+"\n"));


        if (!processEPSR_.waitForFinished(1800000)) return;

        messageText_ += "\nfinished writing "+atoFileName_+" file\n";
        messagesDialog.refreshMessages();
        ui.messagesLineEdit->setText("Finished writing box .ato file");

        //update ui.atoFileTable to include number of mols added for relevant .ato files
        if (container != atoFileName_)
        {
            for (int i = 0; i < ui.atoFileTable->rowCount(); i++)
            {
                if (ui.atoFileTable->item(i,0)->text() == container)
                {
                    ui.atoFileTable->item(i,2)->setText("1");
                }
            }
        }

        for (int i = 0; i < ui.atoFileTable->rowCount(); i++)
        {
            for (int j = 0; j < atoFilesToAdd.count(); j++)
            {
                if (atoFilesToAdd.at(j) == ui.atoFileTable->item(i,0)->text())
                {
                    ui.atoFileTable->item(i,2)->setText(numberOfMolecules.at(j));
                }
            }
        }

        ui.boxAtoLabel->setText(atoFileName_);
        readAtoFileAtomPairs();
        readAtoFileBoxDetails();
        ui.atoAtomList->clear();

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
}

void MainWindow::on_loadBoxButton_clicked (bool checked)
{
    //note in manual that need to manually change moltypeXX to the name of the associated .mol file
    QString atoFile = QFileDialog::getOpenFileName(this, "Choose EPSR box .ato file", workingDir_, tr(".ato files (*.ato)"));
    if (!atoFile.isEmpty())
    {
        QFileInfo fi(atoFile);
        atoFileName_ = fi.fileName();

        ui.boxAtoLabel->setText(atoFileName_);
        readAtoFileAtomPairs();
        readAtoFileBoxDetails();
        ui.atoAtomList->clear();
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
}

bool MainWindow::readAtoFileBoxDetails()
{
    QFile file(workingDir_+atoFileName_);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .ato file.");
        msgBox.exec();
        return false;
    }

    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    line = stream.readLine();
    dataLine = line.split(" ", QString::SkipEmptyParts);
    QString numberMol = dataLine.at(0);
    if (dataLine.count() == 3)
    {
        double boxLength = dataLine.at(1).toDouble();
        QString boxLengthstr;
        boxLengthstr.sprintf("%.4f", boxLength);
        ui.boxAtoLengthA->setText(boxLengthstr);
        ui.boxAtoLengthB->clear();
        ui.boxAtoLengthC->clear();
        ui.boxAtoAxisA->setText("90.0000"); //polar angles in degrees
        ui.boxAtoAxisB->setText("0.0000");
        ui.boxAtoAxisG->setText("0.0000");
//        cryst angles therefore 90,90,90 deg
        double boxVol = boxLength*boxLength*boxLength;
        QString boxVolstr;
        boxVolstr.sprintf("%.2f", boxVol);
        ui.boxAtoVol->setText(boxVolstr);
        ui.temperatureLineEdit->setText(dataLine.at(2));
    }
    else
    {
        ui.temperatureLineEdit->setText(dataLine.at(1));

        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        double boxa = dataLine.at(0).toDouble();
        double boxb = dataLine.at(1).toDouble();
        double boxc = dataLine.at(2).toDouble();
        QString boxastr;
        boxastr.sprintf("%.4f", boxa);
        QString boxbstr;
        boxbstr.sprintf("%.4f", boxb);
        QString boxcstr;
        boxcstr.sprintf("%.4f", boxc);
        ui.boxAtoLengthA->setText(boxastr);
        ui.boxAtoLengthB->setText(boxbstr);
        ui.boxAtoLengthC->setText(boxcstr);       

        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        double boxpb = dataLine.at(0).toDouble();
        double boxtc = dataLine.at(1).toDouble();
        double boxpc = dataLine.at(2).toDouble();
        QString boxpbstr;
        boxpbstr.sprintf("%.4f", boxpb);
        QString boxtcstr;
        boxtcstr.sprintf("%.4f", boxtc);
        QString boxpcstr;
        boxpcstr.sprintf("%.4f", boxpc);
        ui.boxAtoAxisA->setText(boxpbstr);  //polar angles in degrees
        ui.boxAtoAxisB->setText(boxtcstr);
        ui.boxAtoAxisG->setText(boxpcstr);
        //cryst angles therefore:
        // gamma = phib (boxpb)
        // convert all angles to radians
        // bx = cos gammar
        // by = sin gammar
        // phicr = tan-1<2??>(cy,cx)
        // betar = cos-1 cx
        // thetacr = sin-1 (sqrt(cx^2+cy^2))
        // alphar = bos-1 (bc)
        // more maths... need to know what atan2 means in fortran (arc tan in rad but what is the 2??)
        double boxVol = boxa*boxb*sin(boxpb*3.14159265/180)*boxc*cos(boxtc*3.14159265/180);
        QString boxVolstr;
        boxVolstr.sprintf("%.2f", boxVol);
        ui.boxAtoVol->setText(boxVolstr);
    }

    line = stream.readLine();
    dataLine = line.split("  ", QString::SkipEmptyParts);
    ui.atoTetherTolLineEdit->setText(dataLine.at(0));
    ui.intraTransSSLineEdit->setText(dataLine.at(1));
    ui.grpRotSSLineEdit->setText(dataLine.at(2));
    ui.molRotSSLineEdit->setText(dataLine.at(3));
    ui.molTransSSLineEdit->setText(dataLine.at(4));
    ui.vibtempLineEdit->setText(dataLine.at(5));
    ui.angtempLineEdit->setText(dataLine.at(6));
    ui.dihtempLineEdit->setText(dataLine.at(7));

    QStringList tetherMolAtoms;
    QStringList tetherAtoms;
    QStringList tetherList;
    tetherMolAtoms.clear();
    tetherAtoms.clear();
    tetherList.clear();
    int atomctr = 0;
    numberAtomLabels.clear();
    numberAtomLabels.resize(atoAtomLabels.count());
    QRegExp atomLabelrx(" ([A-Z][A-Za-z0-9 ]{2})   ([0-9 ]{1,4})      0");
    QRegExp ecoredcorerx("  ([0-9]{1}[.]{1}[0-9]{5}[E+]{2}[0-9]{2})  ([0-9]{1}[.]{1}[0-9]{5}[E+]{2}[0-9]{2})");
    do {
        line = stream.readLine();
        dataLine = line.split(" ",QString::SkipEmptyParts);
        if (dataLine.count() > 9)
        {
            if (line.contains("T") == true || line.contains("F") == true)
            {
                if (line.contains("T") == true)
                {
                    tetherList.append("T");     //append to list that this is tethered
                    dataLine = line.split(" ",QString::SkipEmptyParts);
                    QString string = dataLine.at(7);
                    QString tetherAtomStr = string.remove("T");
                    int tetherAtom = tetherAtomStr.toInt();
                    tetherAtomStr = QString::number(tetherAtom);
                    tetherAtoms.append(tetherAtomStr);      //append to list which atom/COM tethering is from
                }
                else
                {
                    tetherList.append("F");     //append to list that this is tethered)
                    tetherAtoms.append(" ");
                }

                line = stream.readLine();   //move to next line to check what the first atom of the molecule is
                dataLine = line.split(" ",QString::SkipEmptyParts);
                QString tetherMol = dataLine.at(0);
                if (tetherMolAtoms.contains(tetherMol) == true) //check if atom is already listed
                {
                    tetherList.removeLast();    //if it is, remove T/F and 0000X entries in lists
                    tetherAtoms.removeLast();
                }
                else
                {
                    tetherMolAtoms.append(tetherMol);   //if not, add to list
                }
            }
        }

        if (atomLabelrx.exactMatch(line))
        {
            atomctr++;
            dataLine = line.split("  ", QString::SkipEmptyParts);
            QString data = dataLine.at(0).trimmed();
            for (int i = 0; i < atoAtomLabels.count(); i++)
            {
                if (data == atoAtomLabels.at(i))
                {
                    numberAtomLabels.operator [](i)++; //add 1 to label 'counter' each time find each label
                }
            }
        }
        if (ecoredcorerx.exactMatch(line))
        {
            dataLine = line.split("  ", QString::SkipEmptyParts);
            ui.ecoreLineEdit->setText(dataLine.at(0));
            ui.dcoreLineEdit->setText(dataLine.at(1));
        }
    } while (!line.isNull());
    file.close();
    QString numberAtom = QString::number(atomctr);
    ui.boxAtoMols->setText(numberMol);
    ui.boxAtoAtoms->setText(numberAtom);

    ui.atoTetherTable->setRowCount(tetherMolAtoms.count());
    for (int i = 0; i < tetherMolAtoms.count(); i++)
    {
        ui.atoTetherTable->setItem(i,0, new QTableWidgetItem(tetherMolAtoms.at(i)));
        ui.atoTetherTable->setItem(i,1, new QTableWidgetItem(tetherList.at(i)));
        ui.atoTetherTable->setItem(i,2, new QTableWidgetItem(tetherAtoms.at(i)));
    }

    //calculate atomic number density and input into box
    double totalNumberAtoms = ui.boxAtoAtoms->text().toDouble();
    double boxVolume = ui.boxAtoVol->text().toDouble();
    double numberDensity = totalNumberAtoms/boxVolume;
    QString numberDensityStr;
    numberDensityStr.sprintf("%.6f", numberDensity);
    ui.numberDensityLineEdit->setText(numberDensityStr);
}

bool MainWindow::readAtoFileAtomPairs()
{
    QFile file(workingDir_+atoFileName_);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .ato file.");
        msgBox.exec();
        return false;
    }
    QTextStream stream(&file);
    QString line;
    QRegExp atomPairrx(" ([A-Z][A-Za-z0-9 ]{2}) ([A-Za-z ]{1,2})   ([0-1]{1})");
    atoAtomLabels.clear();

    do {
        line = stream.readLine();
        if (atomPairrx.exactMatch(line))
        {
            atoAtomLabels << atomPairrx.cap(1).trimmed();
        }

    } while (!line.isNull());
    file.close();

    //number of entries in atomLabels list
    const int N_atoAtomLabels = atoAtomLabels.count();

    // Declare and initialise matrix for column to find atom pair partial in...
    ij.initialise(N_atoAtomLabels,N_atoAtomLabels);

    // Populate it
    int count = 0;
    for (int i = 0; i < N_atoAtomLabels; ++i)
    {
        for (int j = i; j < N_atoAtomLabels; ++j)
        {
            ij.ref(i,j) = count;
            ij.ref(j,i) = count;
            ++count;
        }
    }
    nPartials = count;
    return false;
}

void MainWindow::on_boxCompositionButton_clicked(bool checked)
{
    BoxCompositionDialog boxCompositionDialog(this);

    boxCompositionDialog.show();
    boxCompositionDialog.raise();
    boxCompositionDialog.activateWindow();

    boxCompositionDialog.exec();
}

void MainWindow::on_randomiseButton_clicked(bool checked)
{
        QDir::setCurrent(workingDir_);

        QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

//        processEPSR_.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
        processEPSR_.start(epsrBinDir_+"randomise.exe", QStringList() << workingDir_ << "randomise" << atoBaseFileName);
#else
        processEPSR_.start(epsrBinDir_+"randomise", QStringList() << workingDir_ << "randomise" << atoBaseFileName);
#endif
        if (!processEPSR_.waitForStarted()) return;

        if (!processEPSR_.waitForFinished(1800000)) return;
        messageText_ += "\nfinished randomising box .ato file\n";
        messagesDialog.refreshMessages();
        ui.messagesLineEdit->setText("Finished randomising box");
}

void MainWindow::on_updateAtoFileButton_clicked(bool checked)
{
    double atoTemp = ui.temperatureLineEdit->text().toDouble();
    double vibTemp = ui.vibtempLineEdit->text().toDouble();
    double angTemp = ui.angtempLineEdit->text().toDouble();
    double dihTemp = ui.dihtempLineEdit->text().toDouble();
    double intraTransSS = ui.intraTransSSLineEdit->text().toDouble();
    double grpRotSS = ui.grpRotSSLineEdit->text().toDouble();
    double molRotSS = ui.molRotSSLineEdit->text().toDouble();
    double molTransSS = ui.molTransSSLineEdit->text().toDouble();
    double ecore = ui.ecoreLineEdit->text().toDouble();
    double dcore = ui.dcoreLineEdit->text().toDouble();
    double tetherTol = ui.atoTetherTolLineEdit->text().toDouble();
    QString atoTempstr;
    QString vibTempstr;
    QString angTempstr;
    QString dihTempstr;
    QString intraTransSSstr;
    QString grpRotSSstr;
    QString molRotSSstr;
    QString molTransSSstr;
    QString ecorestr;
    QString dcorestr;
    QString tetherTolstr;
    QStringList tetherMolAtoms;
    QStringList tetherAtoms;
    QStringList tetherList;

    for (int i = 0; i < ui.atoTetherTable->rowCount(); i++)
    {
        tetherMolAtoms.append(ui.atoTetherTable->item(i,0)->text());
        tetherList.append(ui.atoTetherTable->item(i,1)->text());
        int tetherAtom = ui.atoTetherTable->item(i,2)->text().toInt();
        tetherAtoms.append(QString("%1").arg(tetherAtom, 5, 10, QChar('0')));
    }

    QDir::setCurrent(workingDir_);

    QFile fileRead(workingDir_+atoFileName_);
    QFile fileWrite(workingDir_+"temp.txt");

    if(!fileRead.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open box.ato file");
        msgBox.exec();
        return;
    }
    if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open temporary file");
        msgBox.exec();
        return;
    }

    QTextStream streamRead(&fileRead);
    QTextStream streamWrite(&fileWrite);
    QString line;
    QStringList dataLine;
    dataLine.clear();
    QString original;

    QRegExp ecoredcorerx("  ([0-9]{1}[.]{1}[0-9]{5}[E+]{2}[0-9]{2})  ([0-9]{1}[.]{1}[0-9]{5}[E+]{2}[0-9]{2})");

    //1st line of ato file
    line = streamRead.readLine();
    dataLine = line.split(" ", QString::SkipEmptyParts);
    if (dataLine.count() == 0) return;
    //for cubic boxes, there are 3 items on the first line - #mols, box length and temp
    if (dataLine.count() == 3)
    {
        streamWrite << "   " << dataLine.at(0) << "  " << dataLine.at(1) << "  " << atoTempstr.setNum(atoTemp,'E',5) << "\n";
    }
    //for non-cubic boxes there are 2 items on the first line - #mols and temp
    //the next 2 lines are the axes lengths and angles
    else
    {
         streamWrite << "   " << dataLine.at(0) << "  " << atoTempstr.setNum(atoTemp,'E',5) << "\n";
         line = streamRead.readLine();
         streamWrite << line << "\n";
         line = streamRead.readLine();
         streamWrite << line << "\n";
    }

    //vibtemp etc line
    line = streamRead.readLine();
    dataLine = line.split(" ", QString::SkipEmptyParts);
    streamWrite << "   " << tetherTolstr.setNum(tetherTol,'E',5)
           << "  " << intraTransSSstr.setNum(intraTransSS,'E',5)
           << "  " << grpRotSSstr.setNum(grpRotSS,'E',5)
           << "  " << molRotSSstr.setNum(molRotSS,'E',5)
           << "  " << molTransSSstr.setNum(molTransSS,'E',5)
           << "  " << vibTempstr.setNum(vibTemp,'E',5)
           << "  " << angTempstr.setNum(angTemp,'E',5)
           << "  " << dihTempstr.setNum(dihTemp,'E',5)
           << "  " << dataLine.at(8)
           << "  " << dataLine.at(9)<< "\n";

    // rest of file
    while (!streamRead.atEnd())
    {
        line = streamRead.readLine();
        dataLine = line.split(" ",QString::SkipEmptyParts);
        //if a tether line, change tethering in accordance with the atoTetherTable (so first atom matches too) **********************TO DO ***************************
        if (dataLine.count() > 9)
        {
            if (ui.atoTetherTable->item(0,1)->text() == "T" && line.contains("F") == true)
            {
                line = line.replace(91, 6, "T"+tetherAtoms.at(0));
            }
            if (ui.atoTetherTable->item(0,1)->text() == "F" && line.contains("T0") == true)
            {
                line = line.replace(91, 6, "F     ");
            }
        }
        original.append(line+"\n");

        if (ecoredcorerx.exactMatch(line))
        {
            original.remove(line+"\n");
            original.append("  "+ecorestr.setNum(ecore,'E',5)+"  "+dcorestr.setNum(dcore,'E',5)+"\n");
        }
    }
    fileWrite.resize(0);
    streamWrite << original;
    fileWrite.close();
    fileRead.close();

    //rename temp file as box .ato file to copy over changes and delete temp file
    QFile::remove(workingDir_+atoFileName_);
    QFile::rename(workingDir_+"temp.txt", workingDir_+atoFileName_);

    messageText_ += "\nbox .ato file updated\n";
    messagesDialog.refreshMessages();
    ui.messagesLineEdit->setText("Finished updating box .ato file");
}

void MainWindow::on_fmoleButton_clicked(bool checked)
{
    int fmoleIter = ui.fmoleLineEdit->text().toInt();

    QDir::setCurrent(workingDir_);

    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

    QProcess processFmole;
    processFmole.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processFmole.startDetached(epsrBinDir_+"fmole.exe", QStringList() << workingDir_ << "fmole" << atoBaseFileName << qPrintable(QString::number(fmoleIter)) << "0");
#else
    processFmole.startDetached(epsrBinDir_+"fmole", QStringList() << workingDir_ << "fmole" << atoBaseFileName << qPrintable(QString::number(fmoleIter)) << "0");
//    processFmole.startDetached("gnome-terminal -e \"bash -c \""+epsrBinDir_+"fmole "+workingDir_+" fmole "+atoBaseFileName+" "+qPrintable(QString::number(fmoleIter))+" 0; exec bash\\\"\"");
#endif
//    if (!processFmole.waitForStarted()) return;

//    if (!processFmole.waitForFinished(1800000)) return;
    messageText_ += "\nfmole is running on box.ato file\n";
    messagesDialog.refreshMessages();
    ui.messagesLineEdit->setText("Running fmole in separate window");
}

void MainWindow::on_viewAtoFileButton_clicked(bool checked)
{
    QString plotCentre = ui.plotAtoCentreLineEdit->text();
    QString maxDist = ui.plotAtoMaxXLineEdit->text()+" "+ui.plotAtoMaxYLineEdit->text()+" "+ui.plotAtoMinZLineEdit->text()+" "+ui.plotAtoMaxZLineEdit->text();
    QString rotCoord = ui.plotAtoRotLineEdit->text();

    QString listExcAtoms;
    QList<QListWidgetItem*> selectedItems = ui.atoAtomList->selectedItems();
    if (selectedItems.count() != 0)
    {
        foreach(QListWidgetItem* item, selectedItems)
        {
            listExcAtoms.append(QString::number((item->data(Qt::UserRole).toInt())+1)+" ");
        }
    }
    else
    {
        listExcAtoms = "0";
    }

    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);
#ifdef _WIN32
    QFile jmolFile(workingDir_+"plot"+atoBaseFileName+".bat");
#else
    QFile jmolFile(workingDir_+"plot"+atoBaseFileName+".sh");
#endif
    if(!jmolFile.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not make Jmol plotting file.");
        msgBox.exec();;
    }
#ifdef _WIN32
    if (plotCentre == "0")
    {
        QTextStream stream(&jmolFile);
        stream << "set EPSRbin=" << epsrBinDir_ << "\n"
               << "set EPSRrun=" << workingDir_ << "\n"
               << "%EPSRbin%plotato.exe " << workingDir_ << " plotato " << atoBaseFileName << " 3" << " "+plotCentre << " "+listExcAtoms << "\n";
        jmolFile.close();
    }
    else
    {
        QTextStream stream(&jmolFile);
        stream << "set EPSRbin=" << epsrBinDir_ << "\n"
               << "set EPSRrun=" << workingDir_ << "\n"
               << "%EPSRbin%plotato.exe " << workingDir_ << " plotato " << atoBaseFileName << " 3" << " "+plotCentre << " "+maxDist << " "+rotCoord << " "+listExcAtoms << "\n";
        jmolFile.close();
    }
#else
    if (plotCentre == "0")
    {
        QTextStream stream(&jmolFile);
        stream << "export EPSRbin=" << epsrBinDir_ << "\n"
               << "export EPSRrun=" << workingDir_ << "\n"
               << "\"$EPSRbin\"'plotato' " << workingDir_ << " plotato " << atoBaseFileName << " 3" << " "+plotCentre << " "+listExcAtoms << "\n";
        jmolFile.close();
    }
    else
    {
        QTextStream stream(&jmolFile);
        stream << "export EPSRbin=" << epsrBinDir_ << "\n"
               << "export EPSRrun=" << workingDir_ << "\n"
               << "\"$EPSRbin\"'plotato' " << workingDir_ << " plotato " << atoBaseFileName << " 3" << " "+plotCentre << " "+maxDist << " "+rotCoord << " "+listExcAtoms << "\n";
        jmolFile.close();
    }
#endif

    QDir::setCurrent(workingDir_);

    QProcess processplotato;
    processplotato.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processplotato.startDetached("plot"+atoBaseFileName+".bat");
#else
    processplotato.startDetached("sh plot"+atoBaseFileName+".sh");
#endif
    ui.messagesLineEdit->setText("Box .ato file plotted in separate window");
}

void MainWindow::on_atoEPSRButton_clicked(bool checked)
{
    QDir::setCurrent(workingDir_);

    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

    QProcess processEditAto;
    processEditAto.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processEditAto.startDetached(epsrBinDir_+"changeato.exe", QStringList() << workingDir_ << "changeato" << atoBaseFileName);
#else
    processEditAto.startDetached(epsrBinDir_+"changeato", QStringList() << workingDir_ << "changeato" << atoBaseFileName);
#endif
    ui.messagesLineEdit->setText("Changeato opened in separate window");
}

bool MainWindow::checkBoxCharge()
{
    double boxCharge = 0;
    for (int i = 0; i < ui.atoFileTable->rowCount(); i++)
    {
        QString molChargeCalcdStr = ui.atoFileTable->item(i,1)->text();
        if (molChargeCalcdStr == "n/a")
        {
            ui.boxAtoCharge->setText("not available");
            return false;
        }
        QString molNumberStr = ui.atoFileTable->item(i,2)->text();
        double molChargeCalcd = QString(molChargeCalcdStr).toDouble();
        int molNumber = QString(molNumberStr).toInt();
        boxCharge = boxCharge + (molChargeCalcd*molNumber);
    }
    QString boxChargeStr;
    boxChargeStr.sprintf("%.4f", boxCharge);
    ui.boxAtoCharge->setText(boxChargeStr);
    return false;
}
