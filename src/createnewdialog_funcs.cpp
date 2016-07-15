#include "createnewdialog.h"
#include "epsrproject.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QString>
#include <QLabel>

CreateNewDialog::CreateNewDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    mainWindow_ = parent;

    QRegExp newEPSRnamerx("[A-Za-z0-9_-]{0,12}");
    ui.lineEditEPSRname->setValidator(new QRegExpValidator(newEPSRnamerx, this));

    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(checkProjectName()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    currentSettingsDir = mainWindow_->epsrDir();
//    currentSettingsDir = QDir::toNativeSeparators(currentSettingsDir);
    ui.lineEditEPSRdir->setText(currentSettingsDir.path());
}

void CreateNewDialog::on_okButton_clicked(bool checked)
{

}

void CreateNewDialog::checkProjectName()
{
    projectName_ = ui.lineEditEPSRname->text();
    QString newDir = workingDir_+"/"+projectName_;
    newDir = QDir::toNativeSeparators(newDir);

    if (QDir(newDir).exists())
    {
        QMessageBox::warning(this, tr("Error creating new EPSR project"),
                             tr("A project with this name already exists, please choose another"));
    }
    else
    {
        accept();
    }
}

void CreateNewDialog::on_cancelButton_clicked(bool checked)
{

}

void CreateNewDialog::on_newBrowseButton_clicked(bool checked)
{
    QString newEPSRdir = QFileDialog::getExistingDirectory(this, "Choose EPSR directory", currentSettingsDir.path());
    if (!newEPSRdir.isEmpty())
    {
        ui.lineEditEPSRdir->setText(newEPSRdir);
        workingDir_ = newEPSRdir+"/run";
        if (!QDir(workingDir_).exists())
        {
            QMessageBox::warning(this, tr("Error creating new EPSR project"),
                                 tr("Check the directory is the main EPSR directory containing the 'run' folder"));
        }
    }
}

QString CreateNewDialog::getEPSRdir()
{
    return ui.lineEditEPSRdir->text();
}

QString CreateNewDialog::getEPSRname()
{
    return ui.lineEditEPSRname->text();
}


