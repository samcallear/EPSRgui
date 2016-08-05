#include <QtGui>
#include <QPushButton>
#include <QDebug>
#include <QProcess>
#include <QString>
#include <QFile>

#include "epsrproject.h"

void MainWindow::on_setupOutButton_clicked(bool checked)
{
    setupOutput();
}

void MainWindow::getOutputType()
{
   outputSetupFileType_ = ui.setupOutTypeComboBox->currentText();
   outputSetupFileType_ = outputSetupFileType_.toLower();
   showAvailableFiles();
}

void MainWindow::showAvailableFiles()
{
    ui.outputAvailableList->clear();

    QDir dir;
    QStringList outputFilter;
    if (outputSetupFileType_== "plot3djmol")
    {
        outputFilter << "*.cube.dat";
    }
    else
    if (outputSetupFileType_== "triangles")
    {
        outputFilter << "*.tri.dat";
    }
    else
    if (outputSetupFileType_== "torangles")
    {
        outputFilter << "*.tor.dat";
    }
    else
    {
        outputFilter << "*."+outputSetupFileType_+".dat";
    }
    QStringList files = dir.entryList(outputFilter, QDir::Files);
    if (files.isEmpty())
    {
        return;
    }

    for (int i = 0; i < files.count(); i++)
    {
        QString fileName = files.at(i).split(".",QString::SkipEmptyParts).at(0);
        ui.outputAvailableList->addItem(fileName);
    }
    ui.outputAvailableList->setCurrentRow(0);

    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);
#ifdef _WIN32
    QFile file(workingDir_+"run"+atoBaseFileName+".bat");
#else
    QFile file(workingDir_+"run"+atoBaseFileName+".sh");
#endif
    if (file.exists() == false)
    {
        return;
    }
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .bat file");
        msgBox.exec();
        return;
    }

    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    do {
        line = stream.readLine();
        if (line.contains(outputSetupFileType_))
        {
            dataLine = line.split(" ", QString::SkipEmptyParts);
            ui.runOutEPSRList->addItem(dataLine.at(3));
        }
    }while (!line.isNull());
    file.close();
}

void MainWindow::setupOutput()
{
    if (ui.setupOutNameNew->text().isEmpty() && ui.outputAvailableList->count() == 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Type a name for the new file");
        msgBox.exec();
        return;
    }

    if (!ui.setupOutNameNew->text().isEmpty())
    {
        outputFileName_ = ui.setupOutNameNew->text();
//        if (!ui.outputAvailableList->contains outputFileName_ then add to the outputAvailableList
        ui.outputAvailableList->addItem(outputFileName_);
    }
    else
    {
        outputFileName_ = ui.outputAvailableList->currentItem()->text();
    }

    ui.setupOutNameNew->clear();

    QDir::setCurrent(workingDir_);

    QProcess processSetupOutput;
    processSetupOutput.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef _WIN32
    processSetupOutput.startDetached(epsrBinDir_+"upset.exe", QStringList() << workingDir_ << "upset" << outputSetupFileType_ << outputFileName_);
#else
    processSetupOutput.startDetached(epsrBinDir_+"upset", QStringList() << workingDir_ << "upset" << outputSetupFileType_ << outputFileName_);
#endif
}

void MainWindow::on_addOutputButton_clicked(bool checked)
{
    if (ui.outputAvailableList->count() == 0) return;
    QString fileToAdd = ui.outputAvailableList->currentItem()->text();
    ui.runOutEPSRList->addItem(fileToAdd);
}

void MainWindow::on_removeOutputButton_clicked(bool checked)
{
    if (ui.runOutEPSRList->count() == 0) return;
    int rowToRemove = ui.runOutEPSRList->currentRow();
    ui.runOutEPSRList->takeItem(rowToRemove);
}

void MainWindow::on_applyOutputsButton_clicked(bool checked)
{
    //make list of file names to be included
    QStringList outputFileNames;
    outputFileNames.clear();

    if (ui.runOutEPSRList->count() != 0)
    {
        for (int i = 0; i < ui.runOutEPSRList->count(); i++)
        {
            QString fileName = ui.runOutEPSRList->item(i)->text();
            outputFileNames.append(fileName);
        }
    }

    //open .bat file
    QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

#ifdef _WIN32
    QFile batFile(workingDir_+"run"+atoBaseFileName+".bat");
#else
    QFile batFile(workingDir_+"run"+atoBaseFileName+".sh");
#endif
    if (batFile.exists() == false)
    {
        QMessageBox msgBox;
        msgBox.setText("The script file doesn't exist yet - run EPSR first");
        msgBox.exec();
    }
    if (!batFile.open(QFile::ReadWrite | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open script file.");
        msgBox.exec();
    }

    QTextStream stream(&batFile);
    QString lineToAdd;
    QString line;
    QString original;
    QStringList dataLine;
    dataLine.clear();

    do {
        line = stream.readLine();
        dataLine = line.split(" ", QString::KeepEmptyParts);
        original.append(line+"\n");
        if (line.contains(outputSetupFileType_)) //if file type is already in the file, remove the line
        {
            original.remove(line+"\n");
        }
        //once get to the 'last' line of the script part, add in the list of filenames to include
        if (!outputFileNames.isEmpty())
        {
#ifdef _WIN32
            if (line.contains("if not exist"))
#else
            if (line.contains("if ([ -e"))
#endif
            {
                original.remove(line+"\n");
                for (int i = 0; i < outputFileNames.count(); i++)
                {
                    QString outputFileName = outputFileNames.at(i);
#ifdef _WIN32
                    lineToAdd = "%EPSRbin%"+outputSetupFileType_+".exe "+workingDir_+" "+outputSetupFileType_+" "+outputFileName+"\n";
#else
                    lineToAdd = "  \"$EPSRbin\"'"+outputSetupFileType_+"' "+workingDir_+" "+outputSetupFileType_+" "+outputFileName+"\n";
#endif
                    original.append(lineToAdd);
                }
                original.append(line+"\n");
            }
        }
    } while (!line.isNull());
    batFile.resize(0);
    stream << original;
    batFile.close();

    ui.messagesLineEdit->setText("script file updated");
}

void MainWindow::outputDlputils()
{
    if (ui.dlputilsOutCheckBox->isChecked())
    {
        //get box side length and divide by 2 to get value for bat file **Alan sugggested add 10% to this to be certain to get any molecule that is right on the very edge of the box
        double boxLengthA = ui.boxAtoLengthA->text().toDouble();
        double boxLengthB = ui.boxAtoLengthB->text().toDouble();
        double boxLengthC = ui.boxAtoLengthC->text().toDouble();
        double halfboxLengthA = boxLengthA/2;
        double halfboxLengthB;
        double halfboxLengthC;
        if (ui.boxAtoLengthB->text().isEmpty())
        {
            halfboxLengthB = halfboxLengthA;
        }
        else
        {
            halfboxLengthB = boxLengthB/2;
        }
        if (ui.boxAtoLengthC->text().isEmpty())
        {
            halfboxLengthC = halfboxLengthA;
        }
        else
        {
            halfboxLengthC = boxLengthC/2;
        }

        QString halfboxLengthAStr = QString::number(halfboxLengthA);
        QString halfboxLengthBStr = QString::number(halfboxLengthB);
        QString halfboxLengthCStr = QString::number(halfboxLengthC);

        //open and edit .bat file
        QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

#ifdef _WIN32
        QFile batFile(workingDir_+"run"+atoBaseFileName+".bat");
#else
        QFile batFile(workingDir_+"run"+atoBaseFileName+".sh");
#endif
        if(batFile.exists() == false)
        {
            QMessageBox msgBox;
            msgBox.setText("The script file doesn't exist yet - run EPSR first");
            msgBox.exec();
        }
        if(!batFile.open(QFile::ReadWrite | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open script file.");
            msgBox.exec();
        }

        QTextStream stream(&batFile);
        QString lineToAdd;
        QString line;
        QString original;
        QStringList dataLine;
        dataLine.clear();

        do {
            line = stream.readLine();
            original.append(line+"\n");
            if (line.contains("writexyz")) //if line is already in the file, remove the line
            {
                original.remove(line+"\n");
            }
#ifdef _WIN32
            if (line.contains("if not exist"))
#else
            if (line.contains("if ([ -e"))
#endif
            {
                original.remove(line+"\n");
#ifdef _WIN32
                lineToAdd = "%EPSRbin%writexyz.exe "+workingDir_+" writexyz "+atoBaseFileName+" y 0 "+halfboxLengthAStr+" "+halfboxLengthBStr+" -"+halfboxLengthCStr+" "+halfboxLengthCStr+" 0 0 0 0"+"\n";
#else
                lineToAdd = "  \"$EPSRbin\"'writexyz' "+workingDir_+" writexyz "+atoBaseFileName+" y 0 "+halfboxLengthAStr+" "+halfboxLengthBStr+" -"+halfboxLengthCStr+" "+halfboxLengthCStr+" 0 0 0 0"+"\n";
#endif
                original.append(lineToAdd);
                original.append(line+"\n");
            }
        } while (!line.isNull());
        batFile.resize(0);
        stream << original;
        batFile.close();

        ui.messagesLineEdit->setText("output for dlputils added to script file");
    }

    else //or check if line exists and remove it if it does
    {
        //open and edit .bat file
        QString atoBaseFileName = atoFileName_.split(".",QString::SkipEmptyParts).at(0);

#ifdef _WIN32
        QFile batFile(workingDir_+"run"+atoBaseFileName+".bat");
#else
        QFile batFile(workingDir_+"run"+atoBaseFileName+".sh");
#endif
        if(batFile.exists() == false)
        {
            QMessageBox msgBox;
            msgBox.setText("The script file doesn't exist yet - run EPSR first");
            msgBox.exec();
        }
        if(!batFile.open(QFile::ReadWrite | QFile::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not open script file.");
            msgBox.exec();
        }

        QTextStream stream(&batFile);
        QString line;
        QString original;
        QStringList dataLine;
        dataLine.clear();

        do {
            line = stream.readLine();
            original.append(line+"\n");
            if (line.contains("writexyz"))
            {
                original.remove(line+"\n");
            }
        } while (!line.isNull());
        batFile.resize(0);
        stream << original;
        batFile.close();

        ui.messagesLineEdit->setText("output for dlputils removed from script file");
    }
}
