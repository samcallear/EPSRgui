#ifndef WTSCOMPONENT
#define WTSCOMPONENT

#include <QString>

class WtsComponent
{
public:

    //constructor
    WtsComponent();

    //data entries
    QString atom;
    QString atsymbol;
    QString iexchange;
    QString isotope1;
    QString abundance1;
    QString isotope2;
    QString abundance2;
//    QString maffq;
//    QString maffdelta;
};

#endif // WTSCOMPONENT

