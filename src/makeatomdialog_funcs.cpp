#include "makeatomdialog.h"
#include "epsrproject.h"

#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QLabel>

MakeAtomDialog::MakeAtomDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    mainWindow_ = parent;

    workingDir_ = mainWindow_->workingDir();

    //restrict what can be entered into each of the boxes
    QRegExp namerx("[A-Z][A-Za-z0-9]{2}");
    ui.nameLineEdit->setValidator(new QRegExpValidator(namerx, this));
    QRegExp numberrx("^\\d*\\.?\\d*$");
    ui.epsilonLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    ui.sigmaLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    ui.massLineEdit->setValidator(new QRegExpValidator(numberrx, this));
    QRegExp chargerx("^-?\\d*\\.?\\d*$");
    ui.chargeLineEdit->setValidator(new QRegExpValidator(chargerx, this));
    QRegExp symbolrx("[A-Z]{1}[a-z]?");
    ui.symbolLineEdit->setValidator(new QRegExpValidator(symbolrx, this));

    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(checkFiles()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void MakeAtomDialog::on_okButton_clicked(bool checked)
{

}

void MakeAtomDialog::checkFiles()
{
    QString fileName = workingDir_+ui.nameLineEdit->text()+".mol";
    if (QFile::exists(fileName) == true)
    {
        QMessageBox::StandardButton msgBox;
        msgBox  = QMessageBox::question(this, "Warning", "This will overwrite the atom .mol and .ato files already present in the project folder with the same name.\nProceed?", QMessageBox::Ok|QMessageBox::Cancel);
        if (msgBox == QMessageBox::Cancel)
        {
            return;
        }
        else
        {
            accept();
        }
    }
    else
    {
        accept();
    }
}

void MakeAtomDialog::on_cancelButton_clicked(bool checked)
{

}

QString MakeAtomDialog::getName()
{
    return ui.nameLineEdit->text();
}

QString MakeAtomDialog::getEpsilon()
{
    return ui.epsilonLineEdit->text();
}

QString MakeAtomDialog::getSigma()
{
    return ui.sigmaLineEdit->text();
}

QString MakeAtomDialog::getMass()
{
    return ui.massLineEdit->text();
}

QString MakeAtomDialog::getCharge()
{
    return ui.chargeLineEdit->text();
}

QString MakeAtomDialog::getSymbol()
{
    return ui.symbolLineEdit->text();
}
