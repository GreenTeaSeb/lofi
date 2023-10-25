#include "launcher.h"
#include "ini_parser.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFile>
#include <QListWidgetItem>
#include <QMimeDatabase>
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

  this->setAttribute(Qt::WA_StyledBackground);
  this->resize(720, 400);
  this->setFixedSize(720, 400);

  // input
  input->setObjectName("input");
  mode->setObjectName("mode");

  set_exec_mode();

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

  QObject::connect(
    list, &QListWidget::itemDoubleClicked, this, &launcher::item_double_click);
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
launcher::item_double_click(QListWidgetItem* item)

{
  start_process(item->data(EXEC_ROLE).toString(),
                item->data(PATH_ROLE).toString());
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

  ini_parser file(path);

  if (file.value("Name") != "") {

    QListWidgetItem* item = new QListWidgetItem();
    QString exec = file.value("Exec");
    QString name = file.value("Name");
    QString icon = file.value("Icon");
    QString type = file.value("Type");
    QStringList searchable = file.value("Categories").split(';');
    searchable.push_front(name);

    if (searchable.filter(search_term, Qt::CaseInsensitive).length() > 0 &&
        list->findItems(name, Qt::MatchExactly).length() == 0) {

      exec.replace(QRegExp(" %."), "");
      item->setData(EXEC_ROLE, exec);
      item->setData(NAME_ROLE, name);
      item->setData(ICON_ROLE, icon);
      item->setData(SEARCHABLE_ROLE, searchable);
      item->setData(TYPE_ROLE, type);
      item->setData(PATH_ROLE, path);
      item->setIcon(get_icon(icon));
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
  if (exec_mode != modes::file) {
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
  } else
    list_files(dir_path, QString::fromStdString(search_word));
}

void
launcher::load_list()
{
  app_list.removeDuplicates();
  if (app_launcher) {
    for (auto& key : most_used) {
      if (list->count() < max_num_of_apps) {
        add_item(key, "");
      }
    }

    for (auto& i : app_list) {
      if (list->count() < max_num_of_apps) {
        add_item(i, "");
      }
    }
  } else {
    for (auto& i : app_list) {
      list->addItem(new QListWidgetItem(QIcon(get_icon(i)), i));
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
            QSettings file(it.fileInfo().absoluteFilePath(),
                           QSettings::IniFormat);

            if (file.allKeys().contains("Desktop Entry/exec",
                                        Qt::CaseInsensitive)) {
              stream << it.fileInfo().absoluteFilePath() << '\n';
              app_list.push_back(it.fileInfo().absoluteFilePath());
            }
          }
        }
      }
    }
    data.close();
  }
}

QIcon
launcher::get_icon(QString name)
{

  if (QIcon::hasThemeIcon(name))
    return QIcon::fromTheme(name);
  else if (QFile(name).exists())
    return QIcon(name);
  else if (default_icon == "")
    return QIcon::fromTheme("application-x-executable");
  else
    return QIcon(QString::fromStdString(default_icon));
}

QIcon
launcher::get_icon_file(QString name)
{
  QString icon_name = QMimeDatabase().mimeTypeForFile(name).iconName();
  if (icon_name.contains("image", Qt::CaseInsensitive))
    return QIcon(name);
  else if (QIcon::hasThemeIcon(icon_name))
    return QIcon::fromTheme(icon_name);
  else
    return QIcon::fromTheme("text-x-generic");
}

void
launcher::exit()
{
  this->close();
};

void
launcher::start_process(QString command, QString path_to_write)
{
  QProcess process(nullptr);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

  // Execution mode
  switch (exec_mode) {

    case modes::exec:
    case modes::file: {
      process.setProgram("/bin/sh");
      process.setArguments(QStringList() << "-c" << command);
      break;
    }
    case modes::term: {
      process.setProgram(default_terminal.c_str());
      QStringList args_full = command.split(" ");
      args_full.prepend("-e");
      process.setArguments(args_full);
      break;
    }
    default: {
      break;
    }
  }
  process.setProcessEnvironment(env);
  process.setWorkingDirectory(getenv("HOME"));

  if (process.startDetached()) {
    if (!path_to_write.isEmpty()) {
      most_used.removeAll(path_to_write);
      most_used.push_front(path_to_write);
      write_most_used();
    }
    exit();
  }
}

void
launcher::execute()
{

  if (app_launcher) {
    if (exec_mode != modes::file) {

      QString command = {};
      QString path = {};
      if (exec_mode == modes::term)
        command = input->text();
      else if (list->currentRow() < 0 &&
               list->count() >= 1) { // If no suggestion is selected
        command = list->item(0)->data(EXEC_ROLE).toString();
        path = list->item(0)->data(PATH_ROLE).toString();

      } else if (list->count() > 0) { // if suggestion is selected
        command = list->currentItem()->data(EXEC_ROLE).toString();
        path = list->currentItem()->data(PATH_ROLE).toString();
      }
      if (!command.isEmpty())
        start_process(command, path);

    } else if (list->currentRow() >= 0) {
      QMimeType type = QMimeDatabase().mimeTypeForFile(
        list->currentItem()->data(PATH_ROLE).toString());

      if (type.inherits("inode/directory")) {
        list_files(list->currentItem()->data(PATH_ROLE).toString(), "");
        input->setText("");
      } else if (type.inherits("application/x-desktop")) {

        ini_parser file(list->currentItem()->data(PATH_ROLE).toString());
        QString exec = file.value("Exec");

        start_process(exec, "");
      } else {
        QDesktopServices::openUrl(
          QUrl::fromLocalFile(list->currentItem()->data(PATH_ROLE).toString()));
        exit();
      }
    }

  } else {
    if (list->currentRow() < 0) { // If no suggestion is selected
      std::cout << input->text().toStdString();
    } else { // if suggestion is selected
      std::cout << list->currentItem()->text().toStdString();
    }

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
      case Qt::Key_K:
      case Qt::Key_Up: {
        if (list_layout == "grid") {
          int step = list->size().width() / list->gridSize().width();
          if (list->currentRow() == -1)
            list->setCurrentRow(list->count() - 1);
          else if (list->currentRow() == list->count() - step - 1)
            list->setCurrentRow(0);
          else
            list->setCurrentRow(list->currentRow() - step);
          break;
        }
      }
      case Qt::Key_L:
      case Qt::Key_Left: {
        if (list->currentRow() == 0)
          list->setCurrentRow(list->count() - 1);
        else
          list->setCurrentRow(list->currentRow() - 1);

        break;
      }
      case Qt::Key_J:
      case Qt::Key_Down: {
        if (list_layout == "grid") {
          int step = list->size().width() / list->gridSize().width();
          if (list->currentRow() == -1)
            list->setCurrentRow(0);
          else if (list->currentRow() == list->count() - step - 1)
            list->setCurrentRow(0);
          else
            list->setCurrentRow(list->currentRow() + step);
          break;
        }
      }
      case Qt::Key_H:
      case Qt::Key_Right: {
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
          if (exec_mode != modes::file) {
            if (list->currentRow() >= 0)
              input->setText(list->currentItem()->data(EXEC_ROLE).toString());
            else
              input->setText(list->item(0)->data(EXEC_ROLE).toString());
          } else {
            if (list->currentRow() >= 0)
              input->setText(list->currentItem()->data(PATH_ROLE).toString());
            else
              input->setText(list->item(0)->data(PATH_ROLE).toString());
          }
        }
        break;
      }
      case Qt::Key_Alt: {
        exec_mode = static_cast<modes>((static_cast<int>(exec_mode) + 1) % 3);
        set_exec_mode();
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

void
launcher::set_exec_mode()
{
  switch (exec_mode) {
    case modes::exec: {
      update_list("");
      mode->setText(QString(" exec") + ": ");
      break;
    }
    case modes::term: {
      update_list("");
      mode->setText(QString(" term") + ": ");
      break;
    }
    case modes::file: {
      mode->setText(QString(" file") + ": ");
      list_files(getenv("HOME"), "");
      break;
    }
    default: {
      break;
    }
  };
}

void
launcher::list_files(QString path, QString search_word)
{

  dir_path = path;

  QDir dir(path);
  dir.setSorting(QDir::Type);
  QList files = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDot);
  list->clear();
  for (auto& file : files) {
    if (file.contains(search_word, Qt::CaseInsensitive)) {
      QListWidgetItem* item = new QListWidgetItem();
      QFileInfo fileinfo(dir, file);
      QMimeType type = QMimeDatabase().mimeTypeForFile(file);

      if (type.inherits("application/x-desktop")) {
        ini_parser ini(fileinfo.absoluteFilePath());
        item->setIcon(get_icon(ini.value("Icon")));
      } else
        item->setIcon(get_icon_file(fileinfo.absoluteFilePath()));
      item->setText(file);

      item->setData(PATH_ROLE, fileinfo.absoluteFilePath());
      if (list_layout == "grid")
        item->setSizeHint(list->sizeHint());
      list->addItem(item);
    }
  }
}
