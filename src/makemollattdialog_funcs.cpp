#include "makemollattdialog.h"
#include "epsrproject.h"

#include <QPushButton>

MakeMolLattDialog::MakeMolLattDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    mainWindow_ = parent;

    ui.atoComboBox->clear();

    QStringList atoFileList = mainWindow_->listAtoFiles();
    QString boxAtoFileName = mainWindow_->atoFileName();

    for (int i = 0; i < atoFileList.count(); i++)
    {
        ui.atoComboBox->addItem(atoFileList.at(i));
    }
    if (!boxAtoFileName.isEmpty())
    {
        ui.atoComboBox->addItem(boxAtoFileName);
    }


    QRegExp integerrx("^\\d*$");
    ui.aLineEdit->setValidator(new QRegExpValidator(integerrx, this));
    ui.bLineEdit->setValidator(new QRegExpValidator(integerrx, this));
    ui.cLineEdit->setValidator(new QRegExpValidator(integerrx, this));

    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(check()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void MakeMolLattDialog::on_okButton_clicked(bool checked)
{

}

void MakeMolLattDialog::on_cancelButton_clicked(bool checked)
{

}

void MakeMolLattDialog::check()
{
    if (ui.aLineEdit->text().isEmpty()
        || ui.bLineEdit->text().isEmpty()
        || ui.cLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Error making .unit file"),
            tr("All fields need to be completed before proceeding."));
        return;
    }
    else
    {
        aCells();
        bCells();
        cCells();
        accept();
    }
}

QString MakeMolLattDialog::getAtoFile()
{
    QString selectedAtoFile = ui.atoComboBox->currentText();
    return selectedAtoFile;
}

int MakeMolLattDialog::aCells()
{
    return ui.aLineEdit->text().toInt();
}

int MakeMolLattDialog::bCells()
{
    return ui.bLineEdit->text().toInt();
}

int MakeMolLattDialog::cCells()
{
    return ui.cLineEdit->text().toInt();
}
