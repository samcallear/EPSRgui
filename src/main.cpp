#include <QApplication>

#include "epsrproject.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("EPSRgui");
    QCoreApplication::setOrganizationDomain("EPSRgui.com");
    QCoreApplication::setApplicationName("EPSRgui");

    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}
