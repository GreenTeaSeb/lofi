#include "launcher.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDirIterator>
#include <QFile>
#include <QListWidgetItem>
#include <QProcess>
#include <QSettings>
#include <QTextStream>
#include <iostream>
#include <stdio.h>

launcher::launcher(QWidget* parent)
  : QWidget(parent)
{
  this->setObjectName("main");
  load_config();
  load_stylesheet();
  parse_arguements();

  Qt::WindowFlags flags = this->windowFlags();
  this->setWindowFlags(flags | Qt::FramelessWindowHint |
                       Qt::WindowStaysOnTopHint);
  this->setAttribute(Qt::WA_StyledBackground);
  this->resize(720, 400);
  this->setMinimumSize(720, 400);

  // input
  input->setObjectName("input");
  mode->setObjectName("mode");

  mode->setText(exec_mode);
  mode->setText(" " + mode->text() + ": ");

  // list for all available apps
  list->setObjectName("list");
  list->setFocusPolicy(Qt::NoFocus);
  list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  list->setLayoutMode(QListWidget::Batched);
  list->setBatchSize(50);
  list->setUniformItemSizes(true);
  list->setMovement(QListWidget::Static);
  if (list_layout == "grid") {
    list->setFlow(QListView::Flow::LeftToRight);
    list->setResizeMode(QListView::Adjust);
    list->setGridSize(QSize(grid_size, grid_size));
    list->setViewMode(QListView::IconMode);
  } else {
    list->setSpacing(5);
  }

  // loading most used

  if (app_launcher) {
    load_most_used();

    list_applications();

  } else {
    std::string line;
    while (std::getline(std::cin, line)) {
      app_list.push_back(QString::fromStdString(line));
    }
  }
  load_list();

  // adding everything
  h_layout->addWidget(mode, 0);
  h_layout->addWidget(input, 10);
  layout->addLayout(h_layout);
  layout->addWidget(list);
  setLayout(layout);
  input->setFocus();
}

void
launcher::parse_arguements()
{
  QCommandLineParser parser;
  QCommandLineOption application_run(
    "r",
    QApplication::translate("main",
                            "list executables from the folders set in config"));

  parser.addOption(application_run);
  parser.process(*QApplication::instance());
  app_launcher = parser.isSet("r");
}

void
launcher::load_stylesheet()
{
  QFile stylesheet_file(QString::fromStdString(config_location + "style.qss"));
  stylesheet_file.open(QFile::ReadWrite | QIODevice::Text);
  stylesheet = stylesheet_file.readAll();
  if (stylesheet.isEmpty()) {
    // setting default
    stylesheet = "#main{\n"
                 "      background: rgb(50, 50, 50);\n"
                 "}\n\n"

                 "#list{\n"
                 "      color: white;\n"
                 "      background:transparent;\n"
                 "      border: 0px;\n"
                 "      font-size: 20px;\n"
                 "}\n\n"

                 "#input{\n"
                 "      color: white;\n"
                 "      background:transparent;\n"
                 "      border: 0px;\n"
                 "      font-size: 30px;\n"
                 "}\n\n"

                 "#mode{\n"
                 "      color: white;\n"
                 "      background:rgba(0,0,0,50);\n"
                 "      border: 0px;\n"
                 "      font-size: 15px;\n"
                 "}\n\n";
    QTextStream out(&stylesheet_file);
    out << stylesheet;
  }
  stylesheet_file.close();
  this->setStyleSheet(stylesheet);
}

void
launcher::add_item(QString path, QString search_term)
{
  QSettings file(path, QSettings::IniFormat);

  file.beginGroup("Desktop Entry");
  if (file.value("Name").toString() != "") {
    QListWidgetItem* item = new QListWidgetItem();
    QString exec = file.value("Exec").toString();
    QString name = file.value("Name").toString();
    QString icon = file.value("Icon").toString();
    QString type = file.value("Type").toString();
    QStringList searchable = file.value("Categories").toString().split(';');
    searchable.push_front(name);
    file.endGroup();

    if (searchable.filter(search_term, Qt::CaseInsensitive).length() > 0 &&
        list->findItems(name, Qt::MatchExactly).length() == 0) {
      item->setData(EXEC_ROLE, exec);
      item->setData(NAME_ROLE, name);
      item->setData(ICON_ROLE, icon);
      item->setData(SEARCHABLE_ROLE, searchable);
      item->setData(TYPE_ROLE, type);
      item->setData(PATH_ROLE, path);

      item->setIcon(get_icon(icon.toStdString()));
      item->setText(name);
      if (list_layout == "grid")
        item->setSizeHint(list->sizeHint());

      list->addItem(item);
    } else
      delete item;
  }
}

void
launcher::update_list(std::string search_word)
{
  list->clear();
  for (auto& key : most_used) {
    if (list->count() < max_num_of_apps) {
      add_item(key, QString::fromStdString(search_word));
    }
  }

  for (auto& i : QStringList(app_list)) {
    if (list->count() < max_num_of_apps) {
      add_item(i, QString::fromStdString(search_word));
    }
  }
}

void
launcher::load_list()
{
  app_list.removeDuplicates();
  for (auto& key : most_used) {
    if (list->count() < max_num_of_apps) {
      add_item(key, "");
    }
  }

  for (auto& i : QStringList(app_list)) {
    if (list->count() < max_num_of_apps) {
      add_item(i, "");
    }
  }
}

void
launcher::list_applications()
{

  QFile data(QString::fromStdString(config_location + "cache"));
  if (data.open(QIODevice::ReadWrite)) {
    QTextStream stream(&data);
    if (data.size() != 0) {
      QString line;
      while (stream.readLineInto(&line)) {
        if (line != "")
          app_list.push_back(line);
      }
    } else {
      for (auto& path : app_locations) {
        if (path != "") {
          QDirIterator it(QString::fromStdString(path),
                          QDir::Files | QDir::NoDot | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
          while (it.hasNext()) {
            it.next();
            stream << it.fileInfo().absoluteFilePath() << '\n';
            app_list.push_back(it.fileInfo().absoluteFilePath());
          }
        }
      }
    }
    data.close();
  }
}

QIcon
launcher::get_icon(std::string app)
{
  QString name = QString::fromStdString(app);

  if (QIcon::hasThemeIcon(name))
    return QIcon::fromTheme(name);
  else if (QFile(name).exists())
    return QIcon(name);
  else if (default_icon == "" &&
           QIcon::hasThemeIcon("application-x-executable"))
    return QIcon::fromTheme("application-x-executable");
  else
    return QIcon(QString::fromStdString(default_icon));
}

void
launcher::exit()
{
  this->close();
};

void
launcher::execute()
{
  if (app_launcher) {
    QProcess process(nullptr);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QString command = {};
    if (list->currentRow() < 0) { // If no suggestion is selected
      command = input->text();
      for (auto& item : list->findItems(input->text(), Qt::MatchExactly)) {
        command = item->data(EXEC_ROLE).toString();
      }
    } else { // if suggestion is selected
      command = list->currentItem()->data(EXEC_ROLE).toString();
    }

    // arguements
    QStringList command_list = command.split(" ");
    QStringList arguments(command_list);
    arguments.removeFirst();
    if (!arguments.isEmpty() && arguments.last().contains('%'))
      arguments.pop_back();

    // Execution mode
    if (strcmp(exec_mode, "exec") == 0) {
      process.setProgram(command_list.at(0));
      process.setArguments(arguments);
    }

    else if (strcmp(exec_mode, "term") == 0) {
      process.setProgram(default_terminal.c_str());
      QStringList args = command_list;
      args.prepend("-e");
      process.setArguments(args);
    }

    process.setProcessEnvironment(env);

    if (process.startDetached()) {
      most_used.push_front(list->currentItem()->data(PATH_ROLE).toString());
      most_used.removeDuplicates();
      write_most_used();
      exit();
    }

  } else {
    std::cout << input->text().toStdString();
    exit();
  }
}

void
launcher::keyPressEvent(QKeyEvent* event)
{
  if (event->modifiers() != Qt::ControlModifier)
    switch (event->key()) {
      case Qt::Key_Return: { // EXECUTE BASH

        try {
          execute();
        } catch (std::exception e) {
          printf("%s", e.what());
        }
        break;
      }
      case Qt::Key_Backspace: { // DELETE CHARACTER FROM INPUT
        if (input->text().size() > 0)
          input->setText(input->text().chopped(1));
        update_list(input->text().toStdString());
        break;
      }
      case Qt::Key_Left:
      case Qt::Key_Up: {
        if (list->currentRow() == 0)
          list->setCurrentRow(list->count() - 1);
        else
          list->setCurrentRow(list->currentRow() - 1);

        break;
      }
      case Qt::Key_Right:
      case Qt::Key_Down: {
        if (list->currentRow() == list->count() - 1)
          list->setCurrentRow(0);
        else
          list->setCurrentRow(list->currentRow() + 1);
        break;
      }
      case Qt::Key_Escape: {
        // list->setCurrentRow(-1);
        exit();
        break;
      }
      case Qt::Key_Tab: {
        if (list->count() > 0) {
          if (list->currentRow() >= 0)
            input->setText(list->currentItem()->text());
          else
            input->setText(list->item(0)->text());
        }
        break;
      }
      case Qt::Key_Alt: {
        if (strcmp(exec_mode, "exec") == 0)
          exec_mode = "term";
        else
          exec_mode = "exec";

        mode->setText(exec_mode);
        mode->setText(" " + mode->text() + ": ");
        break;
      }
      case Qt::Key_Delete: {
        if (list->count() > 0) {
          if (list->currentRow() >= 0) {
            QString to_delete = list->currentItem()->data(PATH_ROLE).toString();
            most_used.removeAll(to_delete);
            update_list(input->text().toStdString().substr(
              0, input->text().toStdString().find(' ')));
            write_most_used();
          }
        }
        break;
      }
      default: {
        input->setText(input->text() + event->text());
        update_list(input->text().toStdString().substr(
          0, input->text().toStdString().find(' ')));
        break;
      }
    }
  else
    switch (event->key()) { // CLEAR CACHE
      case Qt::Key_R: {
        QFile data(QString::fromStdString(config_location + "cache"));
        data.resize(0);
        list->clear();
        app_list.clear();
        load_config();
        list_applications();
        load_list();
        break;
      }

      default: {
        break;
      }
    }
}

void
launcher::write_most_used()
{
  QFile data(QString::fromStdString(config_location + "recents.conf"));
  if (data.open(QIODevice::WriteOnly)) {
    QTextStream out(&data);
    for (auto& key : most_used) {
      if (key != "")
        out << key << '\n';
    }
    data.close();
  }
}

void
launcher::load_most_used()
{
  QFile data(QString::fromStdString(config_location + "recents.conf"));
  if (data.open(QIODevice::ReadOnly)) {
    QTextStream in(&data);

    QString line;
    while (in.readLineInto(&line)) {
      if (line != "")
        most_used.push_back(line);
    }
    data.close();
  }
}

void
launcher::load_config()
{
  QFile data(QString::fromStdString(config_location + "lofi.conf"));
  if (data.open(QIODevice::ReadOnly)) {
    QTextStream in(&data);
    QString line;
    while (in.readLineInto(&line)) {
      if (line != "") {
        QStringList config_line = line.split('=');
        QString parm = config_line.at(0).trimmed();
        QString val = config_line.at(1).trimmed();
        if (configurable.find(parm.toStdString()) != configurable.end()) {
          std::string launcher::*var_pointer =
            configurable.at(parm.toStdString());
          this->*var_pointer = val.toStdString();
        } else {
          if (parm == "check locations") {
            for (auto& path : val.split(',')) {
              app_locations.push_back(path.trimmed().toStdString());
            }
          }
        }
      }
    }
    data.close();
    max_num_of_apps = std::stoi(max_num);
    grid_size = std::stoi(grid_size_stirng);
  }
}

std::string
launcher::to_upper(std::string input)
{
  std::string out = input;
  std::transform(out.begin(), out.end(), out.begin(), ::toupper);
  return out;
}
