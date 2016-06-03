#ifndef MOLFILES_H
#define MOLFILES_H

#include <QList>

class IntraMolecular
{
    public:
    // Atom indices involved in interaction
    int atoms[4];
    // Value of interaction
    double value;
};

#endif // MOLFILES

