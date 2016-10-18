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
    workingDir_ = ui.lineEditEPSRdir->text()+"/";
    workingDir_ = QDir::toNativeSeparators(workingDir_);
    QString newDir = workingDir_+projectName_;
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
    QString newDir = QFileDialog::getExistingDirectory(this, "Choose EPSR directory", currentSettingsDir.path());
    if (!newDir.isEmpty())
    {
        ui.lineEditEPSRdir->setText(newDir);
        workingDir_ = newDir; //this is without the project name
    }
}

QString CreateNewDialog::getPrefDir()
{
    return ui.lineEditEPSRdir->text();
}

QString CreateNewDialog::getEPSRname()
{
    return ui.lineEditEPSRname->text();
}


