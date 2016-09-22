#include "importdialog.h"
#include "epsrproject.h"

#include <QPushButton>
#include <QFileDialog>

ImportDialog::ImportDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    mainWindow_ = parent;

    connect(ui.importButton, SIGNAL(clicked()), this, SLOT(import()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void ImportDialog::on_projectNameBrowseButton_clicked(bool checked)
{

}

void ImportDialog::on_boxAtoBrowseButton_clicked(bool checked)
{

}

void ImportDialog::on_epsrInpBrowseButton_clicked(bool checked)
{

}

void ImportDialog::on_importButton_clicked(bool checked)
{

}

void ImportDialog::on_cancelButton_clicked(bool checked)
{

}

void ImportDialog::import()
{

}
