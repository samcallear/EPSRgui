#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "molfiles.h"
#include "ui_epsrproject.h"
#include "wtscomponent.h"
#include "datafilesettings.h"
#include "array.h"
#include "messagesdialog.h"

class CreateNewDialog;
class PlotDialog;
class MolOptionsDialog;
class SettingsDialog;
class MakeAtomDialog;
class MakeLatticeDialog;
class BoxCompositionDialog;
class AddAtoDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QMainWindow *parent = 0);
    Ui::MainWindow ui;
    int newDialog;
    int setDialog;

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    bool save();
    bool saveAs();
    bool saveCopy();
    void runEPSR();
    void runEPSRcheck();
    void plot();
    void plotEPSRshell();
    void settings();
    void stopEPSR();
    void deleteEPSRinpFile();
    void deleteBoxAtoFile();
    void showMessages();
    void outputfromEPSRprocessReady();

private:
    void createActions();
    bool saveFile();
    void reset();
    void processStart();
    void processEnd();
    void readSettings();

//    CreateNewDialog *createNewDialog;
    PlotDialog *plotDialog;
    MolOptionsDialog *molOptionsDialog;
//    SettingsDialog *settingsDialog;
    MakeAtomDialog *makeAtomDialog;
    MakeLatticeDialog *makeLatticeDialog;
    MessagesDialog messagesDialog;

    QDir exeDir_;
    QDir currentDir;
//    QDir baseDir;
    QString projectName_; //folder name in EPSR but doesn't include path or a "/" after it
    QString workingDir_; //includes projectName and has a "/" after it
    QString epsrDir_; //has a "/" after it
    QString epsrBinDir_; //has a "/" after it
    QString baseFileName_; //use this for the name of the box and the EPSR inp files with the path and without any extensions
    QString molFileName_; //just filename and extension for selected file in molFileList (.mol or .ato), no path
    QString visualiserExe_; //full path and executable
    QString atoFileName_; //just filename and extension for box, no path
    QString dataFileName_; //filename and extension for selected file in dataFileTable, no path
    QString wtsBaseFileName_; //only base file name, not including extension or path for selected file in dataFileTable
    QString dataFileExt_; //data file extension
    QString epsrInpFileName_; //filename and extension for EPSR.inp file
    QFileSystemWatcher epsrFinished_;
    int inpEntries_;
    QByteArray messageText_;
    QProcess processEPSR_;

    int endscript;

public:
    QString workingDir();
    QString atoFileName();
    QDir exeDir();
    QString epsrDir();
    QStringList atomLabels();
    QVector<int> numberOfEachAtomLabel();
    QStringList listAtoFiles();
    QByteArray messageText();

public slots:
    void createNew();
    void open();
    void showPointToolTip1(QMouseEvent *event);
    void showPointToolTip2(QMouseEvent *event);
    void plotZoom1(QWheelEvent *event);
    void plotZoom2(QWheelEvent *event);
    void enableButtons();

    // Mol Files Tab
private slots:
    void on_createMolFileButton_clicked(bool checked);
    void on_molFileLoadButton_clicked(bool checked);
    void on_createAtomButton_clicked(bool checked);
    void on_createLatticeButton_clicked(bool checked);
    void on_removeMolFileButton_clicked(bool checked);
    void on_updateMolFileButton_clicked(bool checked);
    void on_molFileList_itemSelectionChanged();
    void on_viewMolFileButton_clicked(bool checked);
    void setSelectedMolFile();
    void runMolOptions();
    void on_addLJRowAboveButton_clicked(bool checked);
    void on_addLJRowBelowButton_clicked (bool checked);
    void on_deleteLJRowButton_clicked (bool checked);
    void on_addDistRowAboveButton_clicked(bool checked);
    void on_addDistRowBelowButton_clicked (bool checked);
    void on_deleteDistRowButton_clicked (bool checked);
    void on_addAngRowAboveButton_clicked(bool checked);
    void on_addAngRowBelowButton_clicked (bool checked);
    void on_deleteAngRowButton_clicked (bool checked);
    void on_addDihRowAboveButton_clicked(bool checked);
    void on_addDihRowBelowButton_clicked (bool checked);
    void on_deleteDihRowButton_clicked (bool checked);
    void on_deleteDihAllButton_clicked (bool checked);
    void on_addRotRowAboveButton_clicked(bool checked);
    void on_addRotRowBelowButton_clicked (bool checked);
    void on_deleteRotRowButton_clicked (bool checked);
    void on_deleteRotAllButton_clicked (bool checked);
    void on_molChangeAtobutton_clicked(bool checked);
    void on_makeMolExtButton_clicked(bool checked);

public:
    QList<IntraMolecular> bonds;
    int mopacOption;
    int molCharge;
    int molDialog;
    int atomDialog;
    int latticeDialog;
    int nMolFiles;

private:
    bool runjmol();
    bool readMolFile();
    bool updateMolFile();
    bool readAtoFile();
    bool updateAtoFile();

    // Box Ato File Tab
private slots:
    void on_mixatoButton_clicked(bool checked);
    void on_addatoButton_clicked(bool checked);
    void on_loadBoxButton_clicked (bool checked);
    void on_randomiseButton_clicked(bool checked);
    void on_updateAtoFileButton_clicked(bool checked);
    void on_fmoleButton_clicked(bool checked);
    void on_atoEPSRButton_clicked(bool checked);
    void on_viewAtoFileButton_clicked(bool checked);
    void on_boxCompositionButton_clicked(bool checked);

private:
    bool readAtoFileBoxDetails();
    bool readAtoFileAtomPairs();
    bool checkBoxCharge();

public:
    int atoaddDialog;
    QStringList atoAtomLabels;
    QVector<int> numberAtomLabels;
    int nPartials;
    Array2D<int> ij;
    QStringList atoFileList;

    // Wts Files Tab
private slots:
    void on_dataFileBrowseButton_clicked(bool checked);
    void on_dataFileTable_itemSelectionChanged();
    void setSelectedDataFile();
    void on_removeDataFileButton_clicked(bool checked);
    void on_makeWtsButton_clicked(bool checked);

private:
    bool makeNwtsSetup();
    bool readNwtsSetup();
    bool makeXwtsSetup();
    bool readXwtsSetup();
    void makeNwts();
    void makeXwts();
    void refreshDataFileTable();

public:
    QList<WtsComponent> wtscomponents;

    QStringList dataFileList;
    QStringList dataFileTypeList;
    QStringList wtsFileList;
    QStringList normalisationList;

    // Setup EPSR Inp File Tab
private slots:
    void on_setupEPSRButton_clicked(bool checked);
    void on_updateInpPcofFilesButton_clicked(bool checked);
    void on_reloadEPSRinpButton_clicked(bool checked);

private:
    void readEPSRinpFile();
    void updateInpFileTables();
    void updateInpFile();
    bool readEPSRpcofFile();
    void updatePcofFile();
    void updatePcofFileTables();

public:
    QList<DataFileSettings> datafilesettings;
    QStringList inpDescriptions;
    QStringList inpKeywords;
    QStringList inpValues;
    QStringList pcofDescriptions;
    QStringList pcofKeywords;
    QStringList pcofValues;
    QStringList atomPairs;
    QStringList minDistances;

    //epsr outputs
private:
    void setupOutput();
    QString outputFileName_;
    QString outputSetupFileType_;
    QString outputRunFileType_;
    void showAvailableFiles();

private slots:
    void on_setupOutButton_clicked(bool checked);
    void getOutputType();
    void on_addOutputButton_clicked(bool checked);
    void on_removeOutputButton_clicked(bool checked);
    void on_applyOutputsButton_clicked(bool checked);
    void outputDlputils();

    //main window embedded plots
private:
    bool plot1();
    bool getplottype1();
    bool plot2();
    bool getplottype2();
    bool fqplot1();
    bool frplot1();
    bool Eplot1();
    bool Rplot1();
    bool fqplot2();
    bool frplot2();
    bool Eplot2();
    bool Rplot2();
    bool Pplot1();
    bool Pplot2();
    bool Ereqplot1();
    bool Ereqplot2();

private slots:
    void on_plot1Button_clicked();
    void on_plot2Button_clicked();

};

#endif
