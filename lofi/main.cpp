#include <QtWidgets>
#include "launcher.h"
#include <iostream>
int
main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    launcher L;


     L.show();
    return app.exec();
}
