#ifndef DATAFILESETTINGS
#define DATAFILESETTINGS

#include <QString>

class DataFileSettings
{
public:

    //constructor
    DataFileSettings();

    //data entries
    QString datafile;
    QString wtsfile;
    QString nrtype;
    QString rshmin;
    QString szeros;
    QString tweak;
    QString efilereq;
    QString normtype;
};

#endif // DATAFILESETTINGS

