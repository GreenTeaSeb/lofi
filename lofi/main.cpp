#include "launcher.h"
#include <QtWidgets>
#include <iostream>
int
main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  launcher L;
  L.show();

  return app.exec();
  printf("should be closed!");
}
