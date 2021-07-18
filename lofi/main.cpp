#include "launcher.h"
#include <QDebug>
#include <QtWidgets>
int
main(int argc, char* argv[])
{

  QApplication app(argc, argv);
  launcher L;
  L.show();

  return app.exec();
  printf("should be closed!");
}
