#ifndef INI_PARSER_H
#define INI_PARSER_H
#include <QString>
#include <map>
class ini_parser
{
public:
  QString path = {};
  ini_parser(QString path);

  std::map<QString, QString> data;
  QString value(QString name);
};

#endif // INI_PARSER_H
