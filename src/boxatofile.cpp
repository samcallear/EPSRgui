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

void MainWindow::on_mixatoButton_clicked(bool checked)
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
        printf("%s\n", qPrintable(atoFileIndexes.at(i)));
        QString numberMol = ui.atoFileTable->item(i,2)->text();
        numberOfMolecules.append(numberMol);
    }

    int nIndex = atoFileIndexes.count();

    QProcess processMixato;
    processMixato.setProcessChannelMode(QProcess::ForwardedChannels);

    QString projDir = workingDir_;
    projDir = QDir::toNativeSeparators(projDir);
#ifdef _WIN32
    processMixato.start(epsrBinDir_+"mixato.exe", QStringList() << projDir << "mixato");
#else
    processMixato.start(epsrBinDir_+"mixato", QStringList() << workingDir_ << "mixato");
#endif
    if (!processMixato.waitForStarted()) return;

    processMixato.write(qPrintable(QString::number(nIndex)+"\n"));
    QByteArray result = processMixato.readAll();
    qDebug(result);

    for (int i = 0 ; i < nMolFiles; i++)
    {
        int newlines = atoFileIndexes.at(i).toInt();
        for (int nl = 0; nl < newlines; nl++)
        {
            processMixato.write("\n");
            result = processMixato.readAll();
            qDebug(result);
        }
        processMixato.write("y\n");
        result = processMixato.readAll();
        qDebug(result);

        int nMols = numberOfMolecules.at(i).toInt();
        processMixato.write(qPrintable(QString::number(nMols)+"\n"));
        result = processMixato.readAll();
        qDebug(result);
    }
        double numberDensity = ui.numberDensityLineEdit->text().toDouble();
        processMixato.write(qPrintable(QString::number(numberDensity)+"\n"));
        result = processMixato.readAll();
        qDebug(result);

        processMixato.write(qPrintable(atoFileBaseName+"\n"));
        result = processMixato.readAll();
        qDebug(result);

    if (!processMixato.waitForFinished(1800000)) return;

    printf("\nfinished writing %s file\n", qPrintable(atoFileName_));
    ui.messagesLineEdit->setText("Finished writing box .ato file");

    ui.boxAtoLabel->setText(atoFileName_);
    readAtoFileBoxDetails();
    readAtoFileAtomPairs();
    ui.atoAtomList->clear();

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
    ui.atoAsBoxButton->setDisabled(true);
}

void MainWindow::on_addatoButton_clicked(bool checked)
{
    //*****************
    //NOTE
    //This is dependent on order of ato files in the table - the first ato file is the one all subsequent ato files will be added to
    //There are limitations to what will be read during the process - see addato.f for details
    //the atomic number density is irrelevant as the siez opf the box in the first ato file determines the size of the final box.
    //the ecoredcore values at the bottom of the .mol files are important as they determine the overlap during addato
    //tethering of molecules is also important prior to pressing addato.

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
//        printf("%s\n", qPrintable(atoFileIndexes.at(i)));
        QString numberMol = ui.atoFileTable->item(i,2)->text();
        numberOfMolecules.append(numberMol);
    }

    int nIndex = atoFileIndexes.count()-1; //this is the number of files to be added to the first file listed in the table

    QProcess processAddato;
    processAddato.setProcessChannelMode(QProcess::ForwardedChannels);

    QString projDir = workingDir_;
    projDir = QDir::toNativeSeparators(projDir);
#ifdef _WIN32
    processAddato.start(epsrBinDir_+"addato.exe", QStringList() << projDir << "addato");
#else
    processAddato.start(epsrBinDir_+"addato", QStringList() << projDir << "addato");
#endif
    if (!processAddato.waitForStarted()) return;

    processAddato.write(qPrintable(QString::number(nIndex)+"\n"));
    QByteArray result = processAddato.readAll();
    qDebug(result);

    // press enter to get to each ato file listed in the table that will be added to the first ato file in the table
    for (int i = 1 ; i < nMolFiles; i++)
    {
        int newlines = atoFileIndexes.at(i).toInt();
        for (int nl = 0; nl < newlines; nl++)
        {
            processAddato.write("\n");
            result = processAddato.readAll();
            qDebug(result);
        }
        processAddato.write("y\n");
        result = processAddato.readAll();
        qDebug(result);

        int nMols = numberOfMolecules.at(i).toInt();
        processAddato.write(qPrintable(QString::number(nMols)+"\n"));
        result = processAddato.readAll();
        qDebug(result);
    }

    // press enter to get to the ato file that the other files will be added to (this is the first file listed in the table)
    int newlines = atoFileIndexes.at(0).toInt();
    for (int nl = 0; nl < newlines; nl++)
    {
        processAddato.write("\n");
        result = processAddato.readAll();
        qDebug(result);
    }
    processAddato.write("y\n");
    result = processAddato.readAll();
    qDebug(result);

    processAddato.write(qPrintable(atoFileBaseName+"\n"));
    result = processAddato.readAll();
    qDebug(result);

    if (!processAddato.waitForFinished(1800000)) return;

    printf("\nfinished writing %s file\n", qPrintable(atoFileName_));
    ui.messagesLineEdit->setText("Finished writing box .ato file");

    ui.boxAtoLabel->setText(atoFileName_);
    readAtoFileBoxDetails();
    readAtoFileAtomPairs();
    ui.atoAtomList->clear();

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
    ui.atoAsBoxButton->setDisabled(true);
}

void MainWindow::on_atoAsBoxButton_clicked (bool checked)
{
    QMessageBox msgBox;
    msgBox.setText("This is not currently implemented.\n");
    msgBox.exec();
    return;
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
    QString numberMol;

    line = stream.readLine();
    dataLine = line.split("  ", QString::SkipEmptyParts);
    numberMol = dataLine.at(0);
    if (dataLine.count() == 3)
    {
        QString boxLengthstr;
        boxLengthstr = dataLine.at(1);
        ui.boxAtoLengthA->setText(boxLengthstr);
        ui.boxAtoLengthB->clear();
        ui.boxAtoLengthC->clear();
        ui.boxAtoAxisA->setText("90.0000");
        ui.boxAtoAxisB->setText("0.0000");
        ui.boxAtoAxisG->setText("0.0000");
        double boxLength = boxLengthstr.toDouble();
        double boxVol = boxLength*boxLength*boxLength;
        QString boxVolstr;
        boxVolstr.sprintf("%.2f", boxVol);
        ui.boxAtoVol->setText(boxVolstr);
    }
    else
    {
        line = stream.readLine();
        dataLine = line.split("  ", QString::SkipEmptyParts);
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
        double boxtb = dataLine.at(0).toDouble();
        double boxtc = dataLine.at(1).toDouble();
        double boxpb = dataLine.at(2).toDouble();
        QString boxtbstr;
        boxtbstr.sprintf("%.4f", boxtb);
        QString boxtcstr;
        boxtcstr.sprintf("%.4f", boxtc);
        QString boxpbstr;
        boxpbstr.sprintf("%.4f", boxpb);
        ui.boxAtoAxisA->setText(boxtbstr);
        ui.boxAtoAxisB->setText(boxtcstr);
        ui.boxAtoAxisG->setText(boxpbstr);
        double boxVol = boxa*boxb*sin(boxtb*3.14159265/180)*boxc*cos(boxtc*3.14159265/180);
        QString boxVolstr;
        boxVolstr.sprintf("%.2f", boxVol);
        ui.boxAtoVol->setText(boxVolstr);
    }
    file.close();

    ui.boxAtoMols->setText(numberMol);
    ui.boxAtoAtoms->setText("not available");
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
    if (!atomPairrx.isValid()) printf("Could not find atom pairs in ato file.\n");
    atoAtomLabels.clear();

    do {
        line = stream.readLine();
        if (atomPairrx.exactMatch(line))
        {
            atoAtomLabels << atomPairrx.cap(1);
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

void MainWindow::on_randomiseButton_clicked(bool checked)
{
        QDir::setCurrent(workingDir_);

        QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

        QProcess processRandomise;
        processRandomise.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
        processRandomise.start(epsrBinDir_+"randomise.exe", QStringList() << workingDir_ << "randomise" << atoBaseFileName);
#else
        processRandomise.start(epsrBinDir_+"randomise", QStringList() << workingDir_ << "randomise" << atoBaseFileName);
#endif
        if (!processRandomise.waitForStarted()) return;

        if (!processRandomise.waitForFinished(1800000)) return;
        printf("\nfinished randomising box .ato file\n");
        ui.messagesLineEdit->setText("Finished randomising box");
}

void MainWindow::on_updateAtoFileButton_clicked(bool checked)
{
    double atoTemp = ui.temperatureLineEdit->text().toDouble();
    double vibTemp = ui.vibtempLineEdit->text().toDouble();
    double angTemp = ui.angtempLineEdit->text().toDouble();
    double dihTemp = ui.dihtempLineEdit->text().toDouble();
    double ecore = ui.ecoredcoreTable->item(0,0)->text().toDouble();
    double dcore = ui.ecoredcoreTable->item(0,1)->text().toDouble();
    QString atoTempstr;
    QString vibTempstr;
    QString angTempstr;
    QString dihTempstr;
    QString ecorestr;
    QString dcorestr;

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
    //for non cubic boxes there are 2 items on the first line - #mols and temp
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
    streamWrite << "   " << dataLine.at(0)
           << "  " << dataLine.at(1)
           << "  " << dataLine.at(2)
           << "  " << dataLine.at(3)
           << "  " << dataLine.at(4)
           << "  " << vibTempstr.setNum(vibTemp,'E',5)
           << "  " << angTempstr.setNum(angTemp,'E',5)
           << "  " << dihTempstr.setNum(dihTemp,'E',5)
           << "  " << dataLine.at(8)
           << "  " << dataLine.at(9)<< "\n";

    // rest of file
    while (!streamRead.atEnd())
    {
        line = streamRead.readLine();
        original.append(line+"\n");
        if (ecoredcorerx.exactMatch(line))
        {
            original.remove(line+"\n");
            original.append("  "+ecorestr.setNum(ecore,'E',5)+"  "+dcorestr.setNum(dcore,'E',5)+"\n");
//            printf("rx match");
        }
    }
    fileWrite.resize(0);
    streamWrite << original;
    fileWrite.close();
    fileRead.close();

    //rename temp file as box .ato file to copy over changes and delete temp file
    QFile::remove(workingDir_+atoFileName_);
    QFile::rename(workingDir_+"temp.txt", workingDir_+atoFileName_);

    printf("\nbox .ato file updated\n");
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
#endif
//    if (!processFmole.waitForStarted()) return;

//    if (!processFmole.waitForFinished(1800000)) return;
    printf("\nfmole is running on box.ato file\n");
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
    for (int i = 0; i < nMolFiles; i++)
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
