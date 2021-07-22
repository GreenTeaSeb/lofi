#include "ini_parser.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
ini_parser::ini_parser(QString path)
{
  this->path = path;
  QFile data(path);

  if (data.open(QIODevice::ReadOnly)) {
    QTextStream in(&data);
    while (!in.atEnd()) {
      QString line = in.readLine();
      QString parm = line.left(line.indexOf("="));
      QString val = line.section("=", 1);
      this->data.insert({ parm, val });
    }
  }
}

QString
ini_parser::value(QString name)
{
  return data[name];
}
