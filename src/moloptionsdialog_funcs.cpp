#include "moloptionsdialog.h"

#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QLabel>

MolOptionsDialog::MolOptionsDialog(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    ui.mopacComboBox->setCurrentIndex(0);
    ui.molChargeSpinBox->setValue(0);

//    QRegExp molChargerx("^-?[0-9]{1}");
//    ui.molChargeLineEdit->setValidator(new QRegExpValidator(molChargerx, this));

    ui.molOkButton->setEnabled(true);

    connect(ui.molOkButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui.molCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void MolOptionsDialog::on_molOkButton_clicked(bool checked)
{
//    getMolCharge();
//    getMopacOptions();
}

void MolOptionsDialog::on_molCancelButton_clicked(bool checked)
{

}

int MolOptionsDialog::getMolCharge()
{
    return ui.molChargeSpinBox->value();
}

int MolOptionsDialog::getMopacOptions()
{
    return ui.mopacComboBox->currentIndex();
}
