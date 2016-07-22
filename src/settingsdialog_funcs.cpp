#include "settingsdialog.h"
#include "epsrproject.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QString>
#include <QFile>

SettingsDialog::SettingsDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    mainWindow_ = parent;

    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    currentDir=mainWindow_->exeDir();

    readSettings();
}

void SettingsDialog::readSettings()
{
    QString settingsFile = currentDir.path()+"/settings";
    settingsFile = QDir::toNativeSeparators(settingsFile);
    QFile file(settingsFile);
    if (!file.exists()) return;
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open settings file");
        msgBox.exec();
        return;
    }
    QTextStream stream(&file);
    QString line;
    QStringList dataLine;
    dataLine.clear();

    do
    {
        line = stream.readLine();
        dataLine = line.split("  ", QString::SkipEmptyParts);
        if (dataLine.count() != 0)
        {
            if (dataLine.at(0) == "EPSRdir")
            {
                ui.epsrDirlineEdit->setText(dataLine.at(1));
            }
            if (dataLine.at(0) == "visualiser")
            {
                ui.visualiserLineEdit->setText(dataLine.at(1));
            }
        }
    } while (!stream.atEnd());
    file.close();
}

void SettingsDialog::on_okButton_clicked(bool checked)
{
    writeSettingsFile();
}

void SettingsDialog::on_cancelButton_clicked(bool checked)
{

}

void SettingsDialog::on_browseEPSRDirButton_clicked(bool checked)
{
    QString prefEPSRdir = QFileDialog::getExistingDirectory(this, "Choose EPSR directory", currentDir.path());
    ui.epsrDirlineEdit->setText(prefEPSRdir);
}

void SettingsDialog::on_browseVisualiserExeButton_clicked(bool checked)
{
#ifdef _WIN32
    QString prefVisExe = QFileDialog::getOpenFileName(this, "Choose visualiser exectuable", currentDir.path(), tr(".exe files (*.exe)"));
#else
    QString prefVisExe = QFileDialog::getOpenFileName(this, "Choose visualiser exectuable", currentDir.path(), tr("All files (*.*)"));
#endif
    ui.visualiserLineEdit->setText(prefVisExe);
}

QString SettingsDialog::getPrefEPSRdir()
{
    return ui.epsrDirlineEdit->text();
}

QString SettingsDialog::getPrefVisExe()
{
    return ui.visualiserLineEdit->text();
}

void SettingsDialog::writeSettingsFile()
{
    QString settingsFile = currentDir.path()+"/settings";
    QFile file(settingsFile);
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not write to settings file");
        msgBox.exec();
        return;
    }
    QTextStream streamWrite(&file);

    if (!ui.epsrDirlineEdit->text().isEmpty())
    {
        streamWrite << "EPSRdir  " << ui.epsrDirlineEdit->text() << "\n";
    }
    if (!ui.visualiserLineEdit->text().isEmpty())
    {
        streamWrite << "visualiser  " << ui.visualiserLineEdit->text() << "\n";
    }
    file.resize(0);
    file.close();
}
