#include "makelatticedialog.h"
#include "epsrproject.h"

#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QLabel>
#include <QTableWidget>

MakeLatticeDialog::MakeLatticeDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);   

    mainWindow_ = parent;

    workingDir_ = mainWindow_->workingDir();

    //restrict what can be entered into each of the boxes
    QRegExp namerx("[A-Za-z0-9]{9}");
    ui.fileNameLineEdit->setValidator(new QRegExpValidator(namerx, this));
    QRegExp numberrx("^\\d*\\.?\\d*$");
    ui.aLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    ui.alLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    ui.bLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    ui.beLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    ui.cLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    ui.gaLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    ui.ecoreLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    ui.dcoreLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    QRegExp integerrx("^\\d*$");
    ui.aLattLineEdit->setValidator(new QRegExpValidator(integerrx, this));
    ui.bLattLineEdit->setValidator(new QRegExpValidator(integerrx, this));
    ui.cLattLineEdit->setValidator(new QRegExpValidator(integerrx, this));

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(checkEntries()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui.coordTable, SIGNAL(cellChanged(int, int)), this, SLOT(checkAtomLabel(int, int)));
}

void MakeLatticeDialog::on_browseButton_clicked(bool checked)
{
    QString unitFileName = QFileDialog::getOpenFileName(this, "Choose .unit file", workingDir_, tr(".unit files (*.unit)"));
    if (!unitFileName.isEmpty())
    {
        ui.unitFileLineEdit->setText(unitFileName);
        QFileInfo fi(unitFileName);
        QString unitBaseFileName = fi.fileName();
        unitBaseFileName = unitBaseFileName.split(".",QString::SkipEmptyParts).at(0);
        ui.fileNameLineEdit->setText(unitBaseFileName);
        unitFileName_ = unitFileName;
        readUnitFile();
    }
}

void MakeLatticeDialog::on_okButton_clicked(bool checked)
{

}

void MakeLatticeDialog::checkEntries()
{
    QString fileName = workingDir_+ui.fileNameLineEdit->text()+".ato";
    //if anything not filled in give error message
    if (ui.fileNameLineEdit->text().isEmpty()
        || ui.aLineEdit->text().isEmpty()
        || ui.alLineEdit->text().isEmpty()
        || ui.bLineEdit->text().isEmpty()
        || ui.beLineEdit->text().isEmpty()
        || ui.cLineEdit->text().isEmpty()
        || ui.gaLineEdit->text().isEmpty()
        || ui.ecoreLineEdit->text().isEmpty()
        || ui.dcoreLineEdit->text().isEmpty()
        || ui.aLattLineEdit->text().isEmpty()
        || ui.bLattLineEdit->text().isEmpty()
        || ui.cLattLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Error making .unit file"),
            tr("All fields need to be completed before proceeding."));
    }
    else
    if (QFile::exists(fileName) == true)
    {
        QMessageBox::StandardButton msgBox;
        msgBox  = QMessageBox::question(this, "Warning", "This will overwrite the lattice file already present in the project folder with the same name.\nProceed?", QMessageBox::Ok|QMessageBox::Cancel);
        if (msgBox == QMessageBox::Cancel)
        {
            return;
        }
        else
        {
            saveToUnitFile();
            accept();
        }
    }
    else
    {
        saveToUnitFile();
        accept();
    }
}

void MakeLatticeDialog::on_cancelButton_clicked(bool checked)
{

}

bool MakeLatticeDialog::readUnitFile()
{
    QFile file(unitFileName_);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .unit file");
        msgBox.exec();
        return 0;
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
        return 0;
    }
    ui.aLineEdit->setText(dataLine.at(0));
    ui.bLineEdit->setText(dataLine.at(1));
    ui.cLineEdit->setText(dataLine.at(2));
    line = stream.readLine();
    dataLine = line.split(" ", QString::SkipEmptyParts);
    ui.alLineEdit->setText(dataLine.at(0));
    ui.beLineEdit->setText(dataLine.at(1));
    ui.gaLineEdit->setText(dataLine.at(2));
    line = stream.readLine();
    if (line.contains("fractional"))
    {
        ui.fracRadioButton->setChecked(true);
        ui.absRadioButton->setChecked(false);
    }
    else
    {
        ui.absRadioButton->setChecked(true);
        ui.fracRadioButton->setChecked(false);
    }
    line = stream.readLine();
    int numberAtoms = line.toInt();
    ui.coordTable->setRowCount(numberAtoms);
    ui.coordTable->verticalHeader()->setVisible(false);
    QStringList atomLabels;
    atomLabels.clear();
    int ctr = 0;
    for ( int i = 0; i < numberAtoms; i++)
    {
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        ui.coordTable->setItem(i,0, new QTableWidgetItem(dataLine.at(0)));
        ui.coordTable->setItem(i,1, new QTableWidgetItem(dataLine.at(1)));
        ui.coordTable->setItem(i,2, new QTableWidgetItem(dataLine.at(2)));
        ui.coordTable->setItem(i,3, new QTableWidgetItem(dataLine.at(3)));
        //if dataLine.at(0) is a new atomLabel, increase ctr
        if (!atomLabels.contains(dataLine.at(0)))
        {
            ctr++;
            atomLabels.append(dataLine.at(0));
        }
    }
    int numberAtomTypes = ctr;
    ui.paramTable->setRowCount(numberAtomTypes);
    ui.paramTable->verticalHeader()->setVisible(false);
    for (int i = 0; i < numberAtomTypes; i++)
    {
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        ui.paramTable->setItem(i,0, new QTableWidgetItem(dataLine.at(0)));
        ui.paramTable->setItem(i,1, new QTableWidgetItem(dataLine.at(1)));
        ui.paramTable->setItem(i,2, new QTableWidgetItem(dataLine.at(2)));
        line = stream.readLine();
        dataLine = line.split(" ", QString::SkipEmptyParts);
        ui.paramTable->setItem(i,3, new QTableWidgetItem(dataLine.at(0)));
        ui.paramTable->setItem(i,4, new QTableWidgetItem(dataLine.at(1)));
        ui.paramTable->setItem(i,5, new QTableWidgetItem(dataLine.at(2)));
        ui.paramTable->setItem(i,6, new QTableWidgetItem(dataLine.at(3)));
        ui.paramTable->setItem(i,7, new QTableWidgetItem(dataLine.at(4)));
    }
    line = stream.readLine();
    dataLine = line.split(" ", QString::SkipEmptyParts);
    ui.ecoreLineEdit->setText(dataLine.at(0));
    ui.dcoreLineEdit->setText(dataLine.at(1));

    file.close();
}

void MakeLatticeDialog::on_addRowButton_clicked (bool checked)
{
    int row = 0;

    if (ui.coordTable->rowCount() != 0 )
    {
       row = ui.coordTable->currentRow()+1;
    }
    ui.coordTable->insertRow(row);

}

void MakeLatticeDialog::on_deleteRowButton_clicked (bool checked)
{
    if (ui.coordTable->rowCount() == 0) return;
    int row = ui.coordTable->currentRow();

    QTableWidgetItem *item = ui.coordTable->item(row,0);
    if (!item || item->text().isEmpty())
    {
        ui.coordTable->removeRow(row);
        return;
    }

    QString atomLabel = ui.coordTable->item(row,0)->text();
    if (atomLabel.isEmpty()) return;

    ui.coordTable->removeRow(row);

    //make a table of the remaining atom labels and see it the atom label is still present
    QStringList atomLabels;
    atomLabels.clear();
    for (int i = 0; i < ui.coordTable->rowCount(); i++)
    {
        atomLabels.append(ui.coordTable->item(i,0)->text());
    }
    if (atomLabels.contains(atomLabel)) return;

    //if the last instance of the atom label has been removed, then remove this atom label from the paramTable too
    atomLabels.clear();
    for (int i = 0; i < ui.paramTable->rowCount(); i++)
    {
        atomLabels.append(ui.paramTable->item(i,0)->text());
    }
    if (atomLabels.contains(atomLabel))
    {
        QRegExp atomLabelrx(atomLabel);
        int paramRow = atomLabels.indexOf(atomLabelrx);
        ui.paramTable->removeRow(paramRow);
    }
}

void MakeLatticeDialog::checkAtomLabel(int row, int column)
{
    if (column == 0) //only do something if the 1st column is changed
    {
        QString atomLabel = ui.coordTable->item(row,column)->text(); //take the atom label from the first column of the row that has been clicked
        //make a list of all the atom labels present in the paramTable
        if (ui.paramTable->rowCount() != 0)
        {
            QStringList atomLabels;
            atomLabels.clear();
            for (int i = 0; i < ui.paramTable->rowCount(); i++)
            {
                atomLabels.append(ui.paramTable->item(i,0)->text());
            }
            //if the atomLabel just added to the coordTable is present in the paramTable list, do nothing.
            if (atomLabels.contains(atomLabel)) return;
        }
        //otherwise add a row to the paramTable and insert the atomLabel
        int paramRow =0;
        if (ui.paramTable->rowCount() != 0)
        {
            paramRow = ui.paramTable->rowCount();
        }
        ui.paramTable->insertRow(paramRow);
        ui.paramTable->setItem(paramRow,column, new QTableWidgetItem(atomLabel));
    }
    else return;
}

bool MakeLatticeDialog::saveToUnitFile()
{
    QString unitFileName = ui.fileNameLineEdit->text();
    QFile file(workingDir_+unitFileName+".unit");
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open .unit file");
        msgBox.exec();
        return 0;
    }

    QTextStream streamWrite(&file);

    QString coordType;
    if (ui.fracRadioButton->isChecked() == true)
    {
        coordType = "fractional";
    }
    else (coordType = "absolute");
    QString numberAtoms = QString::number(ui.coordTable->rowCount());

    streamWrite << ui.aLineEdit->text() << " " << ui.bLineEdit->text() << " " << ui.cLineEdit->text() << "\n"
                << ui.alLineEdit->text() << " " << ui.beLineEdit->text() << " " << ui.gaLineEdit->text() << "\n"
                << coordType << "\n"
                << numberAtoms << "\n";
    for (int i = 0; i < ui.coordTable->rowCount(); i++)
    {
        streamWrite << ui.coordTable->item(i,0)->text() << " " << ui.coordTable->item(i,1)->text() << " " << ui.coordTable->item(i,2)->text() << " " << ui.coordTable->item(i,3)->text() << "\n";
    }
    for (int i = 0; i < 2; i++)
    {
        streamWrite << ui.paramTable->item(i,0)->text() << " " << ui.paramTable->item(i,1)->text() << " " << ui.paramTable->item(i,2)->text() << "\n"
                    << ui.paramTable->item(i,3)->text() << " " << ui.paramTable->item(i,4)->text() << " " << ui.paramTable->item(i,5)->text() << " " << ui.paramTable->item(i,6)->text() << " " << ui.paramTable->item(i,7)->text() << "\n";
    }
    streamWrite << ui.ecoreLineEdit->text() << " " << ui.dcoreLineEdit->text() << "\n";
    file.close();
    unitFileName_= workingDir_+unitFileName+".unit";
}

QString MakeLatticeDialog::cellsAlongA()
{
    return ui.aLattLineEdit->text();
}

QString MakeLatticeDialog::cellsAlongB()
{
    return ui.bLattLineEdit->text();
}

QString MakeLatticeDialog::cellsAlongC()
{
    return ui.cLattLineEdit->text();
}

QString MakeLatticeDialog::unitFileName()
{
    QFileInfo fi(unitFileName_);
    QString unitFileName = fi.fileName();
    //this gives XXX.unit without the path but with the extension
    return unitFileName;
}

QStringList MakeLatticeDialog::atomTypes()
{
    QStringList atomTypes;
    for (int i = 0; i < ui.paramTable->rowCount(); i++)
    {
        atomTypes.append(ui.paramTable->item(i,0)->text());
    }
    return atomTypes;
}

QString MakeLatticeDialog::charge()
{
//  NOT USED - NOT SURE IF WORKING?**********************************************************
    double molChargeCalcd = 0;
    for (int i = 0; i < ui.paramTable->rowCount(); i++)
    {
        int ctr = 0;
        for (int n = 0; n < ui.coordTable->rowCount(); n++)
        {
            if (ui.coordTable->item(n,0) == ui.paramTable->item(i,0))
            {
                ctr++;
            }
        }
        QString atomTypeChargeStr = ui.paramTable->item(i,6)->text();
        double atomTypeCharge = QString(atomTypeChargeStr).toDouble();
        molChargeCalcd = molChargeCalcd+(ctr*atomTypeCharge);
    }
    QString lattCharge;
    lattCharge.sprintf("%.4f", molChargeCalcd);
    return lattCharge;
}

int MakeLatticeDialog::useAsBox()
{
    int useAsBoxChecked = 0;
    if (ui.lattAsBoxCheckBox->isChecked())
    {
        useAsBoxChecked = 1;
    }
    return useAsBoxChecked;
}
