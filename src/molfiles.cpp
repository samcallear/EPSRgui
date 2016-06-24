#include <QtGui>
#include <QPushButton>
#include <QDebug>
#include <QProcess>
#include <QString>
#include <QTableWidget>

#include "epsrproject.h"
#include "molfiles.h"
#include "moloptionsdialog.h"
#include "makeatomdialog.h"
#include "makelatticedialog.h"

void MainWindow::on_createMolFileButton_clicked(bool checked)
{
    runMolOptions();
}

void MainWindow::runMolOptions()
{
    if (!molOptionsDialog)
    {
        molOptionsDialog = new MolOptionsDialog(this);
    }

    molOptionsDialog->show();
    molOptionsDialog->raise();
    molOptionsDialog->activateWindow();

    molDialog = molOptionsDialog->exec();

    if (molDialog == MolOptionsDialog::Accepted)
    {
        molCharge = molOptionsDialog->getMolCharge();
        mopacOption = molOptionsDialog->getMopacOptions();
        runjmol();
    }
}

bool MainWindow::runjmol()
{
    ui.messagesLineEdit->setText("Running Jmol");

    QProcess processjmol;
    QString program = epsrBinDir_+"Jmol.jar";

    // set jmol save directory to EPSR directory
    QDir::setCurrent(workingDir_);
    processjmol.start("java", QStringList() << "-jar" << program);
    //linux: java -jar ~/src/EPSR25/bin/Jmol.jar
//    if (!processjmol.waitForStarted()) return false;
    if (!processjmol.waitForFinished(1800000))
    {
        ui.messagesLineEdit->setText("Jmol timed out");
        return false;
    }
    //find most recently modified *.jmol file
    QDir dir;
    dir.setSorting(QDir::Time);
    QStringList jmolFilter;
    jmolFilter << "*.jmol";
    QStringList jmolFiles = dir.entryList(jmolFilter, QDir::Files);
    if (jmolFiles.isEmpty())
    {
        ui.messagesLineEdit->setText("No .jmol files found");
        return false;
    }
    QString jmolFileName = jmolFiles.at(0);

    QFileInfo jmolFileInfo(jmolFileName);
    QDateTime jmolModTime;
    jmolModTime = jmolFileInfo.lastModified();
    QDateTime dateTimeNow = QDateTime::currentDateTime();
    if (jmolModTime < dateTimeNow.addSecs(-30))
    {
        ui.messagesLineEdit->setText("No new .jmol files found");
        return false;
    }

    ui.messagesLineEdit->setText("Making .mol and .ato files");  

    QString mopacOptionStr = QString::number(mopacOption);
    QString molChargeStr = QString::number(molCharge);

    // Run MOPAC if mopacOption is selected
    if (mopacOption != 0)
    {
        QString jmolBaseFileName=jmolFileName.split(".",QString::SkipEmptyParts).at(0);
#ifdef _WIN32
        QFile jmolBatFile(workingDir_+"mopac"+jmolBaseFileName+".bat");
#else
        QFile jmolBatFile(workingDir_+"mopac"+jmolBaseFileName+".sh");
#endif
        if(!jmolBatFile.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not make MOPAC setup file.");
            msgBox.exec();;
        }

        QTextStream stream(&jmolBatFile);
#ifdef _WIN32
        stream << "set EPSRbin=" << epsrBinDir_ << "\n"
               << "set EPSRrun=" << workingDir_ << "\n"
               << "%EPSRbin%readmole.exe " << workingDir_ << " readmole .jmol " << jmolBaseFileName << "\n";
#else
        stream << "export EPSRbin=" << epsrBinDir_ << "\n"
               << "export EPSRrun=" << workingDir_ << "\n"
               << "\"$EPSRbin\"'readmole' " << workingDir_ << " readmole .jmol " << jmolBaseFileName << "\n";
#endif
        jmolBatFile.close();

        QProcess processMol;
        processMol.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
        processMol.start("mopac"+jmolBaseFileName+".bat");
#else
        processMol.start("sh mopac"+jmolBaseFileName+".sh");
#endif
        //linux:
        if (!processMol.waitForStarted()) return false;

        processMol.write("3\n");          // select bonding patterns and changelabel section
        QByteArray result = processMol.readAll();
        qDebug(result);

        processMol.write("y\n");          //yes to running mopac
        result = processMol.readAll();
        qDebug(result);

        processMol.write(qPrintable(mopacOptionStr+"\n"));    //mopac option number
        result = processMol.readAll();
        qDebug(result);

        processMol.write(qPrintable(molChargeStr+"\n"));      //molecular charge
        result = processMol.readAll();
        qDebug(result);

        if (!processMol.waitForFinished()) return false;


//        QString jmolBaseFileName = jmolFileName.split(".",QString::SkipEmptyParts).at(0);
//        QFile(mopacFile)(workingDir_+jmolBaseFileName+".log");
//        if(!mopacFile.open(QFile::WriteOnly | QFile::Text))
//        {
//            QMessageBox msgBox;
//            msgBox.setText("Could not create mopac instruction file.");
//            msgBox.exec();
//            return false;
//        }

//        QTextStream streamWrite(&mopacFile);

//        streamWrite << " " <<  mopacOptionStr << "   " << "MMOK SYMMETRY PRECISE GEO-OK CHARGE=" << molChargeStr << "\n"
//                    << " " << workingDir_+jmolBaseFileName << "\n"
//                    << " " << "Run to minimize energy\n";
//        mopacFile.close();

//        //also need to make another file with some more details in it ******************************************

//        QProcess processMopac;
//        processMopac.setProcessChannelMode(QProcess::ForwardedChannels);
//        processMopac.start(epsrBinDir_+"MOPAC_7.exe", QStringList() << workingDir_+jmolBaseFileName);
//        if (!processMopac.waitForStarted()) return false;
//        if (!processMopac.waitForFinished()) return false;

//        printf("\nMOPAC finished - check %s.out file for results of calculation.\n", qPrintable(jmolBaseFileName));
    }
    else
    {
        QProcess processMol;
        processMol.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
        processMol.start(epsrBinDir_+"readmole.exe", QStringList() << workingDir_ << "jmolfile "+jmolFileName);
#else
        processMol.start(epsrBinDir_+"readmole", QStringList() << workingDir_ << "jmolfile "+jmolFileName);
        //linux: ~/src/EPSR25/bin/readmole ~/src/EPSR25/run/test2/ jmolfile molecule.jmol
#endif
        if (!processMol.waitForStarted()) return false;

        processMol.write("3\n");          // select bonding patterns and changelabel section
        QByteArray result = processMol.readAll();
        qDebug(result);

        processMol.write("n\n");    //don't run mopac here as calls to "%EPSRbin%" in path which obviously isn't defined!
        result = processMol.readAll();
        qDebug(result);

        if (!processMol.waitForFinished()) return false;
    }

    // if a new .mol file is created, load this into ui.molFileList:
    QDir dir2;
    dir2.setSorting(QDir::Time);
    QStringList molFilter;
    molFilter << "*.mol";
    QStringList molFiles = dir.entryList(molFilter, QDir::Files);
    if (molFiles.isEmpty())
    {
        ui.messagesLineEdit->setText("No .mol files found");
        return 0;
    }
    QString molFileName = molFiles.at(0);
    QFileInfo molFileInfo(molFileName);
    QDateTime molModTime;
    molModTime = molFileInfo.lastModified();
    QDateTime dateTimeNow2 = QDateTime::currentDateTime();
    if (molModTime < dateTimeNow2.addSecs(-30))
    {
        ui.messagesLineEdit->setText("No new .mol files found");
        return 0;
    }
    molFileName_= molFileName;
    QString cropped_fileName = molFileName_.split(".",QString::SkipEmptyParts).at(0);
    QString atoFileName = cropped_fileName+".ato";

    //update mol file table
    ui.molFileList->QListWidget::addItem(molFileName);
    nMolFiles = ui.molFileList->count();
    ui.molFileList->setCurrentRow(nMolFiles-1);


    //update ato file table in box ato tab
    ui.atoFileTable->setRowCount(nMolFiles);
    if (nMolFiles > 0)
    {
        QTableWidgetItem *item = new QTableWidgetItem(atoFileName);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui.atoFileTable->setItem(nMolFiles-1,0, item);
        ui.atoFileTable->setItem(nMolFiles-1,2, new QTableWidgetItem("0"));
    }

    ui.messagesLineEdit->setText("Finished making .mol and .ato files");

    // update mol bond distance/angles/etc tables
    return readMolFile();

}

void MainWindow::on_molFileLoadButton_clicked(bool checked)
{
    QDir::setCurrent(workingDir_);

    QString molFile = QFileDialog::getOpenFileName(this, "Choose EPSR .mol file", workingDir_, tr(".mol files (*.mol)"));
    if (!molFile.isEmpty())
    {
//        workingDir = QFileInfo
//        //check if molFileName is in the working directory, if not copy it to the working directory with the .ato file

//        if (QFile::exists(workingDir_+"/"+molfilename) == 0)
//        {
//             QFile::copy(molFileName, workingdir+molfilename);

             QFileInfo fi(molFile);
             QString molFileName = fi.fileName();
             molFileName_= molFileName;
             QString cropped_fileName = molFileName.split(".",QString::SkipEmptyParts).at(0);
             QString atoFileName = cropped_fileName+".ato";
//             QFile::copy(atoFileName, workingdir+atofilename);
//        }

        //check if equivalent .ato file also exists and, if not, run makemole to generate it
        if (QFile::exists(atoFileName) == 0)
        {
            QProcess processMakemole;
            processMakemole.setProcessChannelMode(QProcess::ForwardedChannels);

            QString projDir = workingDir_;
            projDir = QDir::toNativeSeparators(projDir);
#ifdef _WIN32
            processMakemole.start(epsrBinDir_+"makemole.exe", QStringList() << projDir << "makemole" << cropped_fileName);
#else
            processMakemole.start(epsrBinDir_+"makemole", QStringList() << projDir << "makemole" << cropped_fileName);
            //linux: ~/src/EPSR25/bin/makemole ~/src/EPSR25/run/test2/ makemole molecule
#endif
            if (!processMakemole.waitForStarted()) return;
            if (!processMakemole.waitForFinished()) return;
        }

        //update mol file table
        ui.molFileList->QListWidget::addItem(molFileName);
        int lastMolFile = ui.molFileList->QListWidget::count();
        ui.molFileList->setCurrentRow(lastMolFile-1);
        nMolFiles = ui.molFileList->count();

        //update ato file table in box ato tab
        ui.atoFileTable->setRowCount(nMolFiles);
        if (nMolFiles > 0)
        {
            QTableWidgetItem *item = new QTableWidgetItem(atoFileName);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
//            item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            ui.atoFileTable->setItem(nMolFiles-1,0, item);
            ui.atoFileTable->setItem(nMolFiles-1,2, new QTableWidgetItem("0"));
        }

        // update mol bond distance/angles/etc tables
        readMolFile();
    }
}

void MainWindow::on_createAtomButton_clicked(bool checked)
{
    if (!makeAtomDialog)
    {
        makeAtomDialog = new MakeAtomDialog(this);
    }

    makeAtomDialog->show();
    makeAtomDialog->raise();
    makeAtomDialog->activateWindow();

    atomDialog = makeAtomDialog->exec();

    if (atomDialog == MakeAtomDialog::Accepted)
    {
        QString atomName = makeAtomDialog->getName();
        QString atomEpsilon = makeAtomDialog->getEpsilon();
        QString atomSigma = makeAtomDialog->getSigma();
        QString atomMass = makeAtomDialog->getMass();
        QString atomCharge = makeAtomDialog->getCharge();
        QString atomSymbol = makeAtomDialog->getSymbol();
        QString atomTemp = makeAtomDialog->getTemp();
        QString atomDensity = makeAtomDialog->getDensity();

        QProcess processMakeAto;
        processMakeAto.setProcessChannelMode(QProcess::ForwardedChannels);
        QString projDir = workingDir_;
        projDir = QDir::toNativeSeparators(projDir);
#ifdef _WIN32
        processMakeAto.start(epsrBinDir_+"makeato.exe", QStringList() << projDir << "makeato" );
#else
        processMakeAto.start(epsrBinDir_+"makeato", QStringList() << projDir << "makeato" );
        //linux: ~/src/EPSR25/bin/makeato ~/src/EPSR25/run/test2/ makeato
#endif
        if (!processMakeAto.waitForStarted()) return;

        processMakeAto.write(qPrintable(atomName+"\n"));
        QByteArray result = processMakeAto.readAll();
        qDebug(result);

        processMakeAto.write(qPrintable(atomEpsilon+"\n"));
        result = processMakeAto.readAll();
        qDebug(result);

        processMakeAto.write(qPrintable(atomSigma+"\n"));
        result = processMakeAto.readAll();
        qDebug(result);

        processMakeAto.write(qPrintable(atomMass+"\n"));
        result = processMakeAto.readAll();
        qDebug(result);

        processMakeAto.write(qPrintable(atomCharge+"\n"));
        result = processMakeAto.readAll();
        qDebug(result);

        processMakeAto.write(qPrintable(atomSymbol+"\n"));
        result = processMakeAto.readAll();
        qDebug(result);

        processMakeAto.write(qPrintable(atomTemp+"\n"));
        result = processMakeAto.readAll();
        qDebug(result);

        processMakeAto.write(qPrintable(atomDensity+"\n"));
        result = processMakeAto.readAll();
        qDebug(result);

        if (!processMakeAto.waitForFinished()) return;

        printf("\nmakeato finished\n");
        QString atoFileName = atomName+".ato";
        QString molFileName = atomName+".mol";
        molFileName_= workingDir_+molFileName;

        //make an equivalent .mol file

        QFile fileRead(workingDir_+atoFileName);
        if(!fileRead.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open atom .ato file");
            msgBox.exec();
            return;
        }

        QFile fileWrite(molFileName_);
        if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not create .mol file");
            msgBox.exec();
            return;
        }

        QTextStream streamRead(&fileRead);
        QTextStream streamWrite(&fileWrite);
        QString lineRead;
        QStringList dataLineRead;
        dataLineRead.clear();

        lineRead = streamRead.readLine();
        lineRead = streamRead.readLine();
        lineRead = streamRead.readLine();
        lineRead = streamRead.readLine();
        lineRead = streamRead.readLine();

        streamWrite << "  .gmol           0\n"
                    << " atom            1  001   " << lineRead << "           0\n"
                    << "changelabel 001 " << atomName << "\n";

        lineRead = streamRead.readLine();
        lineRead = streamRead.readLine();
        lineRead = streamRead.readLine();
        lineRead = streamRead.readLine();
        dataLineRead=lineRead.split(" ", QString::SkipEmptyParts);
        if (dataLineRead.count() == 0) return;

        streamWrite << "potential " << atomName << " " << dataLineRead.at(0) << "  " << dataLineRead.at(1) << "  " << dataLineRead.at(2) << "  " << dataLineRead.at(3) << " " << atomSymbol << "\n"
                    << "temperature " << atomTemp << "\n"
                    << "vibtemp  0.650000E+02\n"
                    << "angtemp  0.100000E+01\n"
                    << "dihtemp  0.100000E+02\n"
                    << "ecoredcore    1.00000    2.00000\n"
                    << "density  " << atomDensity << "\n";

        fileRead.close();
        fileWrite.close();

        //update mol file table
        ui.molFileList->QListWidget::addItem(molFileName);
        int lastMolFile = ui.molFileList->QListWidget::count();
        ui.molFileList->setCurrentRow(lastMolFile-1);
        nMolFiles = ui.molFileList->count();

        //update ato file table in box ato tab
        ui.atoFileTable->setRowCount(nMolFiles);
        if (nMolFiles > 0)
        {
            QTableWidgetItem *item = new QTableWidgetItem(atoFileName);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            ui.atoFileTable->setItem(nMolFiles-1,0, item);
            ui.atoFileTable->setItem(nMolFiles-1,2, new QTableWidgetItem("0"));
        }

        //update LJ etc tables
        readMolFile();

        ui.messagesLineEdit->setText("Finished making new atom");
    }
}

void MainWindow::on_createLatticeButton_clicked(bool checked)
{
    //make the .ato file for the lattice (the lattice can only be single atoms as 'molecules' not multiatom moleucles)
    if (!makeLatticeDialog)
    {
        makeLatticeDialog = new MakeLatticeDialog(this);
    }

    makeLatticeDialog->show();
    makeLatticeDialog->raise();
    makeLatticeDialog->activateWindow();

    latticeDialog = makeLatticeDialog->exec();

    if (latticeDialog == MakeLatticeDialog::Accepted)
    {
        QString unitFileName = makeLatticeDialog->unitFileName(); //this has the .unit extension on it
        QString aLatt = makeLatticeDialog->cellsAlongA();
        QString bLatt = makeLatticeDialog->cellsAlongB();
        QString cLatt = makeLatticeDialog->cellsAlongC();

        QProcess processMakeLattice;
        processMakeLattice.setProcessChannelMode(QProcess::ForwardedChannels);
        QString projDir = workingDir_;
        projDir = QDir::toNativeSeparators(projDir);
#ifdef _WIN32
        processMakeLattice.start(epsrBinDir_+"makelattice.exe", QStringList() << projDir << "makelattice" );
#else
        processMakeLattice.start(epsrBinDir_+"makelattice", QStringList() << projDir << "makelattice" );
#endif
        if (!processMakeLattice.waitForStarted()) return;

        processMakeLattice.write(qPrintable(unitFileName+"\n"));
        QByteArray result = processMakeLattice.readAll();
        qDebug(result);

        processMakeLattice.write(qPrintable(aLatt+" "+bLatt+" "+cLatt+"\n"));
        result = processMakeLattice.readAll();
        qDebug(result);

        processMakeLattice.write(qPrintable(unitFileName+"\n"));
        result = processMakeLattice.readAll();
        qDebug(result);

        if (!processMakeLattice.waitForFinished()) return;

        printf("\nmakelattice finished\n");

        //make the .mol files for each atom in the lattice
        QStringList atomTypes = makeLatticeDialog->atomTypes(); //get list of atom labels from paramTable

        //read in .unit file to get epsilon, sigma, amu, charge, atom symbol for each atom type and write out to each .mol file
        QFile file(workingDir_+unitFileName);
        if(!file.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open .unit file");
            msgBox.exec();
            return;
        }

        QTextStream stream(&file);
        QString line;
        QStringList dataLine;
        dataLine.clear();

        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        if (dataLine.count() == 0)
        {
            QMessageBox msgBox;
            msgBox.setText("Could not read .unit file");
            msgBox.exec();
            return;
        }
        line = stream.readLine();
        line = stream.readLine();
        line = stream.readLine();
        int numberAtoms = line.toInt();
        for (int i = 0; i < numberAtoms; i++)
        {
            line = stream.readLine();
        }

        for (int i = 0; i < atomTypes.count(); i++)
        {
            QString molFileName = atomTypes.at(i);
            QFile fileWrite(workingDir_+molFileName+".mol");
            if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
            {
                QMessageBox msgBox;
                msgBox.setText("Could not create .mol file");
                msgBox.exec();
                return;
            }

            line = stream.readLine();
            dataLine = line.split(" ", QString::SkipEmptyParts);
            QString symbol = dataLine.at(1);
            line = stream.readLine();
            dataLine = line.split(" ", QString::SkipEmptyParts);
            QString epsilon = dataLine.at(0);
            QString sigma = dataLine.at(1);
            QString mass = dataLine.at(2);
            QString charge = dataLine.at(3);

            QTextStream streamWrite(&fileWrite);
            streamWrite << "  .gmol           0\n"
                        << " atom            1  001     0.00000E+00  0.00000E+00  0.00000E+00           0\n"
                        << "changelabel 001 " << atomTypes.at(i) << "\n"
                        << "potential " << atomTypes.at(i) << " " << epsilon << " " << sigma << " " << mass << " " << charge << " " << symbol << "\n"
                        << "temperature 300\n"
                        << "vibtemp  0.650000E+02\n"
                        << "angtemp  0.100000E+01\n"
                        << "dihtemp  0.100000E+02\n"
                        << "ecoredcore    0.00000    1.00000\n"
                        << "density  0.1\n";
            fileWrite.close();

            QProcess processMakemole;
            processMakemole.setProcessChannelMode(QProcess::ForwardedChannels);

            QString projDir = workingDir_;
            projDir = QDir::toNativeSeparators(projDir);
#ifdef _WIN32
            processMakemole.start(epsrBinDir_+"makemole.exe", QStringList() << projDir << "makemole" << molFileName);
#else
            processMakemole.start(epsrBinDir_+"makemole", QStringList() << projDir << "makemole" << molFileName);
#endif
            if (!processMakemole.waitForStarted()) return;
            if (!processMakemole.waitForFinished()) return;

            ui.molFileList->QListWidget::addItem(molFileName+".mol");
        }
        file.close();

        //update lattice.ato with correct .mol files at bottom
        QString atoFileName = unitFileName.split(".", QString::SkipEmptyParts).at(0);
        QFile fileato(workingDir_+atoFileName+".ato");
        if(!fileato.open(QFile::ReadWrite | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open lattice .ato file");
            msgBox.exec();
            return;
        }

        QTextStream streamato(&fileato);
        QString lineato;
        QStringList dataLineato;
        dataLineato.clear();
        QString originalato;
        int ctr_moltype = 0;

        while (!streamato.atEnd())
        {
            lineato = streamato.readLine();
            originalato.append(lineato+"\n");
            if (lineato.isEmpty()) break;
            if (lineato.contains("moltype"))
            {
                originalato.remove(lineato+"\n");
                dataLineato = lineato.split(" ", QString::SkipEmptyParts);
                originalato.append("   "+dataLineato.at(0)+"  "+atomTypes.at(ctr_moltype)+"  "+dataLineato.at(2)+"  "+dataLineato.at(3)+"  "+dataLineato.at(4)+"  "+dataLineato.at(5)+"\n");
                ctr_moltype++;
            }
        }
        fileato.resize(0);
        streamato << originalato;
        fileato.close();

        //update ato file table
        int lastAtoFile = ui.atoFileTable->rowCount();
        ui.atoFileTable->insertRow(lastAtoFile);
        QTableWidgetItem *item = new QTableWidgetItem(atoFileName+".ato");
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui.atoFileTable->setItem(lastAtoFile,0, item);
        //THIS ISNT WORKING **********************************************************************************
        QString lattCharge = makeLatticeDialog->charge();
        item = new QTableWidgetItem(lattCharge);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui.atoFileTable->setItem(lastAtoFile,1, item);
//        ui.atoFileTable->setItem(lastAtoFile,1, new QTableWidgetItem(makeLatticeDialog->charge()));
        ui.atoFileTable->setItem(lastAtoFile,2, new QTableWidgetItem("0"));

        //set mol file table to select last item and update (runs via 'selectionChanged' signal
        nMolFiles = ui.molFileList->count();
        ui.molFileList->setCurrentRow(nMolFiles-1);

        //update LJ etc tables
       // readMolFile();

        ui.messagesLineEdit->setText("Finished making new lattice");
    }
}

void MainWindow::on_makeMolExtButton_clicked(bool checked)
{
    if (visualiserExe_.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("Please set a preferred visualiser in Settings.");
        msgBox.exec();
        return;
    }

    QDir::setCurrent(workingDir_);

    QProcess processAten;
    processAten.setProcessChannelMode(QProcess::ForwardedChannels);
    processAten.startDetached(visualiserExe_, QStringList() << atoFileName_);
    ui.messagesLineEdit->setText("Preferred visualiser started");
}

void MainWindow::setSelectedMolFile()
{
    int row = ui.molFileList->currentRow();
    QString molFileName = ui.molFileList->item(row)->text();
    molFileName_ = molFileName;

    readMolFile();
}

void MainWindow::on_viewMolFileButton_clicked(bool checked)
{    
    if (!visualiserExe_.isEmpty())
    {
        QProcess processAten;
        QString cropped_fileName = molFileName_.split(".",QString::SkipEmptyParts).at(0);
        QString atoFileName = cropped_fileName+".ato";
        processAten.setProcessChannelMode(QProcess::ForwardedChannels);
        processAten.startDetached(visualiserExe_, QStringList() << atoFileName);
        ui.messagesLineEdit->setText(".mol file opened");
    }
    else
    {
        QString atoBaseFileName = molFileName_.split(".",QString::SkipEmptyParts).at(0);

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

        QTextStream stream(&jmolFile);
#ifdef _WIN32
        stream << "set EPSRbin=" << epsrBinDir_ << "\n"
               << "set EPSRrun=" << workingDir_ << "\n"
               << "%EPSRbin%plotato.exe " << workingDir_ << " plotato " << atoBaseFileName << " 3 0 0\n";
#else
        stream << "export EPSRbin=" << epsrBinDir_ << "\n"
               << "export EPSRrun=" << workingDir_ << "\n"
               << "\"$EPSRbin\"'plotato' " << workingDir_ << " plotato " << atoBaseFileName << " 3 0 0\n";
#endif
        jmolFile.close();

        QDir::setCurrent(workingDir_);

        QProcess processplotato;
        processplotato.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
        processplotato.startDetached("plot"+atoBaseFileName+".bat");
#else
        processplotato.startDetached("sh plot"+atoBaseFileName+".sh");
#endif
        ui.messagesLineEdit->setText(".mol file plotted in separate window");

/*
        QDir::setCurrent(workingDir_);

        QProcess processplotato;
        processplotato.setProcessChannelMode(QProcess::ForwardedChannels);
        processplotato.startDetached(qPrintable("set EPSRbin="+epsrBinDir_+"\n"));

        processplotato.write(qPrintable("set EPSRrun="+workingDir_+"\n"));
        QByteArray result = processplotato.readAll();
        qDebug(result);

        processplotato.write(qPrintable("%EPSRbin%plotato.exe "+workingDir_+" plotato "+atoBaseFileName+" 3 0 0\n"));
        result = processplotato.readAll();
        qDebug(result);
*/
        ui.messagesLineEdit->setText(".mol file plotted in separate window");
    }
}

void MainWindow::on_removeMolFileButton_clicked(bool checked)
{
    int row = ui.molFileList->currentRow();
    if (ui.molFileList->count() > 1)
    {
            ui.molFileList->takeItem(row);
            setSelectedMolFile();
            ui.atoFileTable->QTableWidget::removeRow(row);
    }
    else
    {
        ui.molFileList->clear();
        ui.atoFileTable->clear();

        ui.molAtomTable->clearContents();
        ui.molBondTable->clearContents();
        ui.molAngleTable->clearContents();
        ui.molDihedralTable->clearContents();
        ui.molRotTable->clearContents();
        ui.molLJTable->clearContents();
    }
    ui.messagesLineEdit->setText(".mol file removed");
}

void MainWindow::on_molFileList_itemSelectionChanged()
{
    if (ui.molFileList->count() !=0 )
    {
        setSelectedMolFile();
    }
}

bool MainWindow::readMolFile()
{
    QFile file(molFileName_);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .mol file");
        msgBox.exec();
        return 0;
    }
    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    QStringList atomLabels;
    QStringList atomTypes;
    QStringList bondAtoms;
    QStringList bondDistances;
    QStringList angleAtoms;
    QStringList bondAngles;
    QStringList dihedralAtoms;
    QStringList dihedrals;
    QStringList rotAtom1;
    QStringList rotAtom2;
    QStringList ljAtoms;
    QStringList sigmas;
    QStringList epsilons;
    QStringList masses;
    QStringList charges;
    QStringList ljTypes;
    QString ecorestr;
    QString dcorestr;
    atomLabels.clear();
    atomTypes.clear();
    bondAtoms.clear();
    bondDistances.clear();
    angleAtoms.clear();
    bondAngles.clear();
    dihedralAtoms.clear();
    dihedrals.clear();
    rotAtom1.clear();
    rotAtom2.clear();
    ljAtoms.clear();
    sigmas.clear();
    epsilons.clear();
    masses.clear();
    charges.clear();
    ljTypes.clear();

    do
    {
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        if (dataLine.count() == 0) break;
        if (dataLine.at(0) == "changelabel")
        {
            atomLabels.append(dataLine.at(1));
            atomTypes.append(dataLine.at(2));
        }
        if (dataLine.at(0) == "bond")
        {
            bondAtoms.append(dataLine.at(1)+" "+dataLine.at(2));
            bondDistances.append(dataLine.at(3));
        }
        if (dataLine.at(0) == "angle")
        {
            angleAtoms.append(dataLine.at(1)+" "+dataLine.at(2)+" "+dataLine.at(3));
            bondAngles.append(dataLine.at(4));
        }
        if (dataLine.at(0) == "dihedral")
        {
            dihedralAtoms.append(dataLine.at(1)+" "+dataLine.at(2)+" "+dataLine.at(3)+" "+dataLine.at(4));
            dihedrals.append(dataLine.at(5));
        }
        if (dataLine.at(0) == "rot")
        {
            rotAtom1.append(dataLine.at(1));
            rotAtom2.append(dataLine.at(2));
        }
        if (dataLine.at(0) == "potential")
        {
            ljAtoms.append(dataLine.at(1));
            sigmas.append(dataLine.at(2));
            epsilons.append(dataLine.at(3));
            masses.append(dataLine.at(4));
            charges.append(dataLine.at(5));
            ljTypes.append(dataLine.at(6));
        }
        if (dataLine.at(0) == "ecoredcore")
        {
            ecorestr=dataLine.at(1);
            dcorestr=dataLine.at(2);
        }

    } while (!line.isNull());
    file.close();

    //read .ato file for .mol to get additional details
    QString atoFile = molFileName_.split(".",QString::SkipEmptyParts).at(0);
    QFile fileato(atoFile+".ato");
    if(!fileato.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .ato file");
        msgBox.exec();
        return 0;
    }
    QTextStream streamato(&fileato);
    QString lineato;
    QStringList dataLineato;
    dataLineato.clear();

    lineato = streamato.readLine();
    dataLineato=lineato.split(" ",QString::SkipEmptyParts);
    if (dataLineato.count() == 2)       //This is for non-cubic .ato files
    {
        lineato = streamato.readLine(); //READ IN OTHER ADDITIONAL DETAILS**********************************
        lineato = streamato.readLine();
        lineato = streamato.readLine();
        lineato = streamato.readLine();
    }
    else                                //this is for cubic .ato files
    {
        lineato = streamato.readLine(); //READ IN OTHER ADDITIONAL DETAILS**********************************
        dataLineato=lineato.split(" ",QString::SkipEmptyParts);
        ui.tolLineEdit->setText(dataLineato.at(0));
        lineato = streamato.readLine();
    }
    if (lineato.contains("T") == true)  //the following depends on the number of items in this line NOT CHANGING!!!!!
    {
        ui.molTetherCheckBox->setChecked(true);
        dataLineato = lineato.split(" ",QString::SkipEmptyParts);
        QString tetherCoordX = dataLineato.at(4);
        ui.tetherCoordXLineEdit->setText(tetherCoordX);
        QString tetherCoordY = dataLineato.at(5);
        ui.tetherCoordYLineEdit->setText(tetherCoordY);
        QString tetherCoordZ = dataLineato.at(6);
        ui.tetherCoordZLineEdit->setText(tetherCoordZ);
        QString string = dataLineato.at(7);
        QString tetherAtomStr = string.remove("T");
        int tetherAtom = tetherAtomStr.toInt();
        tetherAtomStr = QString::number(tetherAtom);
        ui.tetherAtomLineEdit->setText(tetherAtomStr);
    }
    else
    {
        ui.molTetherCheckBox->setChecked(false);
    }
/*
    QRegExp ecoredcorerx("  ([0-9]{1}[.]{1}[0-9]{5}[E+]{2}[0-9]{2})  ([0-9]{1}[.]{1}[0-9]{5}[E+]{2}[0-9]{2})");

    do {
        lineato = streamato.readLine();
        if (ecoredcorerx.exactMatch(lineato))
        {
            dataLineato = lineato.split(" ",QString::SkipEmptyParts);
            ui.molEcoreDcoreTable->setItem(0,0, new QTableWidgetItem(dataLineato.at(0)));
            ui.molEcoreDcoreTable->setItem(0,1, new QTableWidgetItem(dataLineato.at(1)));
        }
    } while (!lineato.isNull());*/
    fileato.close();

    //number of entries lists
    const int N_bondDistances = bondDistances.count();
    const int N_bondAngles = bondAngles.count();
    const int N_dihedrals = dihedrals.count();
    const int N_rotGroups = rotAtom1.count();
    const int N_atomLabels = atomLabels.count();
    const int N_LJparam = ljAtoms.count();

    //show data in Tables
    ui.molAtomTable->setRowCount(N_atomLabels);
    ui.molAtomTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < N_atomLabels; ++i)
    {
        QTableWidgetItem *item = new QTableWidgetItem(atomLabels.at(i));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui.molAtomTable->setItem(i,0, item);
        ui.molAtomTable->setItem(i,1, new QTableWidgetItem(atomTypes.at(i)));
    }

    ui.molBondTable->setRowCount(N_bondDistances);
    ui.molBondTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < N_bondDistances; ++i)
    {
        ui.molBondTable->setItem(i,0, new QTableWidgetItem(bondAtoms.at(i)));
        ui.molBondTable->setItem(i,1, new QTableWidgetItem(bondDistances.at(i)));
    }

    ui.molAngleTable->setRowCount(N_bondAngles);
    ui.molAngleTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < N_bondAngles; ++i)
    {
        ui.molAngleTable->setItem(i,0, new QTableWidgetItem(angleAtoms.at(i)));
        ui.molAngleTable->setItem(i,1, new QTableWidgetItem(bondAngles.at(i)));
    }

    ui.molDihedralTable->setRowCount(N_dihedrals);
    ui.molDihedralTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < N_dihedrals; ++i)
    {
        ui.molDihedralTable->setItem(i,0, new QTableWidgetItem(dihedralAtoms.at(i)));
        ui.molDihedralTable->setItem(i,1, new QTableWidgetItem(dihedrals.at(i)));
    }

    ui.molRotTable->setRowCount(N_rotGroups);
    ui.molRotTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < N_rotGroups; ++i)
    {
        ui.molRotTable->setItem(i,0, new QTableWidgetItem(rotAtom1.at(i)));
        ui.molRotTable->setItem(i,1, new QTableWidgetItem(rotAtom2.at(i)));
    }

    ui.molLJTable->setRowCount(N_LJparam);
    ui.molLJTable->verticalHeader()->setVisible(false);

    for (int i = 0; i < N_LJparam; ++i)
    {
        ui.molLJTable->setItem(i,0, new QTableWidgetItem(ljAtoms.at(i)));
        ui.molLJTable->setItem(i,1, new QTableWidgetItem(sigmas.at(i)));
        ui.molLJTable->setItem(i,2, new QTableWidgetItem(epsilons.at(i)));
        ui.molLJTable->setItem(i,3, new QTableWidgetItem(masses.at(i)));
        ui.molLJTable->setItem(i,4, new QTableWidgetItem(charges.at(i)));
        ui.molLJTable->setItem(i,5, new QTableWidgetItem(ljTypes.at(i)));
    }

    ui.molBondTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.molBondTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.molAngleTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.molAngleTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.molDihedralTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.molDihedralTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.molRotTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.molRotTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.molLJTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.molLJTable->setSelectionMode(QAbstractItemView::SingleSelection);

    ui.molEcoreDcoreTable->setItem(0,0, new QTableWidgetItem(ecorestr));
    ui.molEcoreDcoreTable->setItem(0,1, new QTableWidgetItem(dcorestr));

    //calculate charge for selected .mol file
    double molChargeCalcd = 0;
    for (int i = 0; i < ljAtoms.count(); i++)
    {
        int ctr = 0;
        for (int n = 0; n < atomTypes.count(); n++)
        {
            if (atomTypes.at(n) == ljAtoms.at(i))
            {
                ctr++;
            }
        }
        QString atomTypeChargeStr = charges.at(i);
        double atomTypeCharge = QString(atomTypeChargeStr).toDouble();
        molChargeCalcd = molChargeCalcd+(ctr*atomTypeCharge);
    }
    if (ljAtoms.isEmpty())
    {
        ui.molChargeLabel->setText("not available");
        QTableWidgetItem *item = new QTableWidgetItem("n/a");
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        int row = ui.molFileList->currentRow();
        ui.atoFileTable->setItem(row,1, item);
    }
    else
    {
        QString molChargeCalcdStr;
        molChargeCalcdStr.sprintf("%.4f", molChargeCalcd);
        ui.molChargeLabel->setText(molChargeCalcdStr);
        QTableWidgetItem *item = new QTableWidgetItem(molChargeCalcdStr);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        int row = ui.molFileList->currentRow();
        ui.atoFileTable->setItem(row,1, item);
    }

    return true;
}

bool MainWindow::updateMolFile()
{
    if (ui.molFileList->count() == 0)
    {
        return false;
    }

    QDir::setCurrent(workingDir_);

    QFile fileRead(workingDir_+molFileName_);
    QFile fileWrite(workingDir_+"temp.txt");

    if(!fileRead.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .mol file");
        msgBox.exec();
        return false;
    }
    if(!fileWrite.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open temporary file");
        msgBox.exec();
        return false;
    }

    QTextStream streamRead(&fileRead);
    QTextStream streamWrite(&fileWrite);
    QString line;
    QStringList dataLine;
    dataLine.clear();
    QString original;

    while (!streamRead.atEnd())
    {
        line = streamRead.readLine();
        if(!line.contains("bond")&&!line.contains("angle")
                &&!line.contains("dihedral")&&!line.contains("rot")
                &&!line.contains("changelabel")&&!line.contains("potential")
                &&!line.contains("temperature")&&!line.contains("vibtemp")
                &&!line.contains("angtemp")&&!line.contains("dihtemp")
                &&!line.contains("density")&&!line.contains("ecoredcore"))
        {
            original.append(line+"\n");
        }
    }
    fileWrite.resize(0);
    streamWrite << original;
    fileRead.close();

    if (ui.molBondTable->rowCount() != 0)
    {
        for (int i = 0; i < ui.molBondTable->rowCount(); ++i)
        {
            streamWrite << "bond " << ui.molBondTable->item(i,0)->text() << "    " << ui.molBondTable->item(i,1)->text() << "\n";
        }
    }

    if (ui.molAngleTable->rowCount() != 0)
    {
        for (int i = 0; i < ui.molAngleTable->rowCount(); ++i)
        {
            streamWrite << "angle " << ui.molAngleTable->item(i,0)->text() << "  " << ui.molAngleTable->item(i,1)->text() << "\n";
        }
    }

    if (ui.molDihedralTable->rowCount() != 0 )
    {
        for (int i = 0; i < ui.molDihedralTable->rowCount(); ++i)
        {
            streamWrite << "dihedral  " << ui.molDihedralTable->item(i,0)->text() << "  " << ui.molDihedralTable->item(i,1)->text() << "\n";
        }
    }

    if (ui.molRotTable->rowCount() != 0 )
    {
        for (int i = 0; i < ui.molRotTable->rowCount(); ++i)
        {
            streamWrite << "rot  " << ui.molRotTable->item(i,0)->text() << "  " << ui.molRotTable->item(i,1)->text() << "\n";
        }
    }

    for (int i = 0; i < ui.molAtomTable->rowCount(); ++i)
    {
        QString atomNumber = QString("%1").arg(i+1, 3, 10, QChar('0'));
        streamWrite << "changelabel " << atomNumber << " " << ui.molAtomTable->item(i,1)->text() << "\n";
    }

    for (int i = 0; i < ui.molLJTable->rowCount(); ++i)
    {
        //need to pad atom label with a blank space at end (item 0) and charges with a blank space if '-' not present (item 4)
        streamWrite << "potential " << ui.molLJTable->item(i,0)->text() << "  " << ui.molLJTable->item(i,1)->text() << "  " << ui.molLJTable->item(i,2)->text() << "  " << ui.molLJTable->item(i,3)->text() << " " << ui.molLJTable->item(i,4)->text() << " " << ui.molLJTable->item(i,5)->text() << "\n";
    }
    streamWrite << "temperature  0.300000E+03\n"
              "vibtemp  0.650000E+02\n"
              "angtemp  0.100000E+01\n"
              "dihtemp  0.100000E+01\n";

    QString ecorestr = ui.molEcoreDcoreTable->item(0,0)->text();
    QString dcorestr = ui.molEcoreDcoreTable->item(0,1)->text();

    streamWrite << "ecoredcore    " << ecorestr << "    " << dcorestr << "\n"
              "density  0.100000E-02\n";
    fileWrite.close();

    //rename temp file as .mol file to copy over changes and delete temp file
    fileRead.remove();
    fileWrite.rename(workingDir_+molFileName_);
    printf("finished updating mol file %s\n", qPrintable(molFileName_));

    //run fmole 0 times to make everything is consistent
    QDir::setCurrent(workingDir_);

    QString molBaseFileName = molFileName_.split(".",QString::SkipEmptyParts).at(0);

    QProcess processFmole;
    processFmole.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processFmole.start(epsrBinDir_+"fmole.exe", QStringList() << workingDir_ << "fmole" << molBaseFileName << "0" << "0");
#else
    processFmole.start(epsrBinDir_+"fmole", QStringList() << workingDir_ << "fmole" << molBaseFileName << "0" << "0");
#endif
    if (!processFmole.waitForStarted()) return false;

    if (!processFmole.waitForFinished()) return false;

    //update molecule .ato file with additional details (not those already changed via .mol file)****FINISH DETAILS TO UPDATE*********************
    QString atoFile = molFileName_.split(".",QString::SkipEmptyParts).at(0);
    QFile fileato(workingDir_+atoFile+".ato");
    if(!fileato.open(QFile::ReadWrite | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .ato file");
        msgBox.exec();
        return 0;
    }
    QFile filetemp(workingDir_+"temp");
    if(!filetemp.open(QFile::ReadWrite | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open temporary file");
        msgBox.exec();
        return 0;
    }
    QTextStream streamatoR(&fileato);
    QTextStream streamatoW(&filetemp);
    QString lineato;
    QStringList dataLineato;
    dataLineato.clear();
    QString originalato;
    QString tether;
    int tetherAtom = ui.tetherAtomLineEdit->text().toInt();
    QString tetherAtomStr = QString("%1").arg(tetherAtom, 5, 10, QChar('0'));
    QString tetherCoordX = ui.tetherCoordXLineEdit->text();
    QString tetherCoordY = ui.tetherCoordYLineEdit->text();
    QString tetherCoordZ = ui.tetherCoordZLineEdit->text();
    if (ui.molTetherCheckBox->isChecked() == true)
    {
        tether = "T";
    } else (tether = "F");
//    QRegExp ecoredcorerx("  ([0-9]{1}[.]{1}[0-9]{5}[E+]{2}[0-9]{2})  ([0-9]{1}[.]{1}[0-9]{5}[E+]{2}[0-9]{2})");

    lineato = streamatoR.readLine();
    dataLineato=lineato.split(" ",QString::SkipEmptyParts);
    if (dataLineato.count() == 2)       //this is for non-cubic .ato files
    {
        streamatoW << lineato << "\n";
        lineato = streamatoR.readLine();
        streamatoW << lineato << "\n";
        lineato = streamatoR.readLine();
        streamatoW << lineato << "\n";
    }
    else                                //this is for cubic .ato files
    {
        streamatoW << lineato << "\n";
    }
    lineato = streamatoR.readLine();
    dataLineato = lineato.split(" ",QString::SkipEmptyParts);
    streamatoW << "   " << ui.tolLineEdit->text();  //This is the line starting with tol and contains vibtemp etc
    for (int i = 1; i < dataLineato.count(); i++ )
    {
        streamatoW << "  " << dataLineato.at(i);
    }
    streamatoW << "\n";
    lineato = streamatoR.readLine();
    dataLineato = lineato.split(" ",QString::SkipEmptyParts);
    for (int i = 0; i < 4; i++ )
    {
        streamatoW << "   " << dataLineato.at(i);   //this is the line starting with the number of atoms and containing tether
    }
    streamatoW << "   " << tetherCoordX << "   " << tetherCoordY << "   " << tetherCoordZ << "   " << tether+tetherAtomStr << "               0     1\n";

    do {
        lineato = streamatoR.readLine();
        originalato.append(lineato+"\n");
        if (lineato.contains("moltype01"))
        {
            originalato.remove(lineato+"\n");
            originalato.append("   1 "+atoFile+"  0.000000E+00  0.000000E+00  0.100000E+01  0.100000E+00\n");
        }
    } while (!lineato.isNull());
    fileato.close();

    filetemp.resize(0);
    streamatoW << originalato;
    filetemp.close();

    fileato.remove();
    filetemp.rename(workingDir_+atoFile+".ato");

    //read mol file back into tables and update charge on mol file
    readMolFile();

    printf("\nfinished updating molecule ato file\n");

    ui.messagesLineEdit->setText(".mol file updated");

    return true;
}

void MainWindow::on_addLJRowAboveButton_clicked(bool checked)
{
    int row = 0;
    if (ui.molLJTable->rowCount() != 0 )
    {
       row = ui.molLJTable->currentRow();
    }
    ui.molLJTable->insertRow(row);
}

void MainWindow::on_addLJRowBelowButton_clicked (bool checked)
{
    int row = 0;
    if (ui.molLJTable->rowCount() != 0 )
    {
       row = ui.molLJTable->currentRow()+1;
    }
    ui.molLJTable->insertRow(row);
}

void MainWindow::on_deleteLJRowButton_clicked (bool checked)
{
    int row = ui.molLJTable->currentRow();
    ui.molLJTable->removeRow(row);
}

void MainWindow::on_addDistRowAboveButton_clicked(bool checked)
{
    int row = 0;
    if (ui.molBondTable->rowCount() != 0 )
    {
       row = ui.molBondTable->currentRow();
    }
    ui.molBondTable->insertRow(row);
}

void MainWindow::on_addDistRowBelowButton_clicked (bool checked)
{
    int row = 0;
    if (ui.molBondTable->rowCount() != 0 )
    {
       row = ui.molBondTable->currentRow()+1;
    }
    ui.molBondTable->insertRow(row);
}

void MainWindow::on_deleteDistRowButton_clicked (bool checked)
{
    int row = ui.molBondTable->currentRow();
    ui.molBondTable->removeRow(row);
}

void MainWindow::on_addAngRowAboveButton_clicked(bool checked)
{
    int row = 0;
    if (ui.molAngleTable->rowCount() != 0 )
    {
       row = ui.molAngleTable->currentRow();
    }
    ui.molAngleTable->insertRow(row);
}

void MainWindow::on_addAngRowBelowButton_clicked (bool checked)
{
    int row = 0;
    if (ui.molAngleTable->rowCount() != 0 )
    {
       row = ui.molAngleTable->currentRow()+1;
    }
    ui.molAngleTable->insertRow(row);
}

void MainWindow::on_deleteAngRowButton_clicked (bool checked)
{
    int row = ui.molAngleTable->currentRow();
    ui.molAngleTable->removeRow(row);
}

void MainWindow::on_addDihRowAboveButton_clicked(bool checked)
{
    int row = 0;
    if (ui.molDihedralTable->rowCount() != 0 )
    {
       row = ui.molDihedralTable->currentRow();
    }
    ui.molDihedralTable->insertRow(row);
}

void MainWindow::on_addDihRowBelowButton_clicked (bool checked)
{
    int row = 0;
    if (ui.molDihedralTable->rowCount() != 0 )
    {
       row = ui.molDihedralTable->currentRow()+1;
    }
    ui.molDihedralTable->insertRow(row);
}

void MainWindow::on_deleteDihRowButton_clicked (bool checked)
{
    int row = ui.molDihedralTable->currentRow();
    ui.molDihedralTable->removeRow(row);
}

void MainWindow::on_deleteDihAllButton_clicked (bool checked)
{
    ui.molDihedralTable->setRowCount(0);
}

void MainWindow::on_addRotRowAboveButton_clicked(bool checked)
{
    int row = 0;
    if (ui.molRotTable->rowCount() != 0 )
    {
       row = ui.molRotTable->currentRow();
    }
    ui.molRotTable->insertRow(row);
}

void MainWindow::on_addRotRowBelowButton_clicked (bool checked)
{
    int row = 0;
    if (ui.molRotTable->rowCount() != 0 )
    {
       row = ui.molRotTable->currentRow()+1;
    }
    ui.molRotTable->insertRow(row);
}

void MainWindow::on_deleteRotRowButton_clicked (bool checked)
{
    int row = ui.molRotTable->currentRow();
    ui.molRotTable->removeRow(row);
}

void MainWindow::on_deleteRotAllButton_clicked (bool checked)
{
    ui.molRotTable->setRowCount(0);
}

void MainWindow::on_updateMolFileButton_clicked(bool checked)
{
    updateMolFile();
}

void MainWindow::on_molChangeAtobutton_clicked(bool checked)
{
    QDir::setCurrent(workingDir_);


    QString atoBaseFileName = molFileName_.split(".",QString::SkipEmptyParts).at(0);

    QProcess processEditAto;
    processEditAto.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processEditAto.startDetached(epsrBinDir_+"changeato.exe", QStringList() << workingDir_ << "changeato" << atoBaseFileName);
#else
    processEditAto.startDetached(epsrBinDir_+"changeato", QStringList() << workingDir_ << "changeato" << atoBaseFileName);
#endif
    ui.messagesLineEdit->setText("Changeato opened in separate window");
}
