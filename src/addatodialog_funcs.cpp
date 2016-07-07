#include "addatodialog.h"
#include "epsrproject.h"

#include <QPushButton>

AddAtoDialog::AddAtoDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    mainWindow_ = parent;

    ui.atoComboBox->clear();
    ui.atoTable->clearContents();

    QStringList atoFileList = mainWindow_->listAtoFiles();
    QString boxAtoFileName = mainWindow_->atoFileName();

    for (int i = 0; i < atoFileList.count(); i++)
    {
        ui.atoComboBox->addItem(atoFileList.at(i));
        ui.atoList->addItem(atoFileList.at(i));
    }
    if (!boxAtoFileName.isEmpty())
    {
        ui.atoComboBox->addItem(boxAtoFileName);
    }

    ui.atoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.atoTable->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(checkEntries()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void AddAtoDialog::on_okButton_clicked(bool checked)
{

}

void AddAtoDialog::on_cancelButton_clicked(bool checked)
{

}

void AddAtoDialog::checkEntries()
{
    QList<QTableWidgetItem*> twi = ui.atoTable->findItems("0", Qt::MatchExactly);
    if (ui.atoTable->rowCount() < 1 || twi.isEmpty() == false)
    {
        QMessageBox::warning(this, tr("No .ato files to add"),
            tr("Add ato files to the right hand table and input the number of these molecules to add."));
    }
    else
    {
        accept();
    }
}

void AddAtoDialog::on_addAtoButton_clicked(bool checked)
{
    QString atoFileName = ui.atoList->currentItem()->text();

    int lastAtoFile = ui.atoTable->rowCount();
    ui.atoTable->setRowCount(lastAtoFile+1);
    ui.atoTable->setItem(lastAtoFile,0, new QTableWidgetItem(atoFileName));
    ui.atoTable->setItem(lastAtoFile,1, new QTableWidgetItem("0"));
}

void AddAtoDialog::on_removeAtoButton_clicked(bool checked)
{
    if (ui.atoTable->rowCount() != 0 )
    {
        int row = ui.atoTable->currentRow();
        ui.atoTable->removeRow(row);
    }
}

QString AddAtoDialog::getContainer()
{
    return ui.atoComboBox->currentText();
}

QStringList AddAtoDialog::getAtoFiles()
{
    QStringList selectedAtoFiles;

    for (int i = 0; i < ui.atoTable->rowCount(); i++)
    {
        selectedAtoFiles.append(ui.atoTable->item(i,0)->text());
    }
    return selectedAtoFiles;
}

QStringList AddAtoDialog::getNumberMols()
{
    QStringList numberMols;

    for (int i = 0; i < ui.atoTable->rowCount(); i++)
    {
        numberMols.append(ui.atoTable->item(i,1)->text());
    }
    return numberMols;
}
