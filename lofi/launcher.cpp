#include "launcher.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDirIterator>
#include <QFile>
#include <QListWidgetItem>
#include <QProcess>
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
launcher::update_list(std::string search_word)
{
  list->clear();
  if (search_word == "")
    for (auto& key : most_used) {
      if (list->count() < max_num_of_apps) {
        QFileInfo file(key);
        QListWidgetItem* item = new QListWidgetItem(
          get_icon(file.baseName().toStdString()), file.baseName());
        item->setData(Qt::UserRole, file.absoluteFilePath());
        if (list_layout == "grid")
          item->setSizeHint(list->sizeHint());
        list->addItem(item);
      }
    }

  QStringList shorter_list =
    app_list.filter(QString::fromStdString(search_word), Qt::CaseInsensitive);
  for (auto& i : shorter_list) {
    if (list->count() > max_num_of_apps)
      break;
    QFileInfo file(i);
    if (file.baseName().contains(QString::fromStdString(search_word)) &&
        list->findItems(file.baseName(), Qt::MatchExactly).length() < 1) {
      QListWidgetItem* item = new QListWidgetItem(
        get_icon(file.baseName().toStdString()), file.baseName());
      item->setData(Qt::UserRole, file.absoluteFilePath());
      if (list_layout == "grid")
        item->setSizeHint(list->sizeHint());
      list->addItem(item);
    }
  }
}

void
launcher::load_list()
{
  app_list.removeDuplicates();
  for (auto& key : most_used) {
    if (list->count() < max_num_of_apps) {
      QFileInfo file(key);
      QListWidgetItem* item = new QListWidgetItem(
        get_icon(file.baseName().toStdString()), file.baseName());
      item->setData(Qt::UserRole, file.absoluteFilePath());
      if (list_layout == "grid")
        item->setSizeHint(list->sizeHint());
      list->addItem(item);
    }
  }

  for (auto& i : app_list) {
    if (list->count() < max_num_of_apps) {
      QFileInfo file(i);
      QListWidgetItem* item = new QListWidgetItem(
        get_icon(file.baseName().toStdString()), file.baseName());
      item->setData(Qt::UserRole, file.absoluteFilePath());
      if (list_layout == "grid")
        item->setSizeHint(list->sizeHint());
      list->addItem(item);
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
        QDirIterator it(QString::fromStdString(path),
                        QDir::Files | QDir::Executable | QDir::NoDot |
                          QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
          it.next();
          stream << it.fileInfo().absoluteFilePath() << '\n';
          app_list.push_back(it.fileInfo().absoluteFilePath());
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
  else if (default_icon == "")
    return QIcon::fromTheme("application-x-executable");
  else {
    QIcon icon(QString::fromStdString(default_icon));
    return icon;
  }
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
    std::string command = {};
    if (list->currentRow() < 0) { // If no suggestion is selected
      command = input->text().toStdString();
      if (strcmp(exec_mode, "exec") == 0) {
        for (auto& i : app_list.filter(QString::fromStdString(command),
                                       Qt::CaseInsensitive)) {
          QFileInfo file(i);
          if (file.baseName() == QString::fromStdString(command))
            command = file.absoluteFilePath().toStdString();
        }
      }

    } else { // if suggestion is selected
      command =
        list->currentItem()->data(Qt::UserRole).toString().toStdString();
    }
    QStringList command_list = QString::fromStdString(command).split(" ");
    QStringList arguments(command_list);
    arguments.removeFirst();
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
      QString base_name = QFileInfo(command_list.at(0)).baseName();
      most_used.removeAll(base_name);
      most_used.removeAll(command_list.at(0));

      for (auto& line : most_used) {
        QFileInfo info(line);
        if (info.baseName() == base_name) {
          most_used.removeAll(info.absoluteFilePath());
        }
      }

      most_used.insert(most_used.begin(), command_list.at(0));

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
            QString to_delete =
              list->currentItem()->data(Qt::UserRole).toString();
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
