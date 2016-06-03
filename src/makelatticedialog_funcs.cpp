#include "makelatticedialog.h"

#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QLabel>

MakeLatticeDialog::MakeLatticeDialog(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void MakeLatticeDialog::on_okButton_clicked(bool checked)
{

}

void MakeLatticeDialog::on_cancelButton_clicked(bool checked)
{

}
