#include "launcher.h"
#include <QFile>
#include <QList>
#include <QListWidgetItem>
#include <QProcess>
#include <QTextStream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdio.h>

namespace fs = std::filesystem;

launcher::launcher(QWidget* parent)
  : QWidget(parent)
{
  this->setObjectName("main");
  load_config();
  load_stylesheet();

  Qt::WindowFlags flags = this->windowFlags();
  this->setWindowFlags(flags | Qt::FramelessWindowHint |
                       Qt::WindowStaysOnTopHint);
  this->setAttribute(Qt::WA_StyledBackground);
  this->resize(720, 400);
  this->setMinimumSize(720, 400);

  // loading most used
  load_most_used();

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

  if (list_layout == "grid") {
    list->setFlow(QListView::Flow::LeftToRight);
    list->setResizeMode(QListView::Adjust);
    list->setGridSize(QSize(grid_size, grid_size));
    list->setViewMode(QListView::IconMode);
  } else {
    list->setSpacing(5);
  }

  list_applications(app_locations[0]);
  find_app_icons();
  update_list("");

  // adding everything
  h_layout->addWidget(mode, 0);
  h_layout->addWidget(input, 10);
  layout->addLayout(h_layout);
  layout->addWidget(list);
  setLayout(layout);
  input->setFocus();
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

  for (const auto& key : most_used)
    if (key.find(search_word) != std::string::npos &&
        list->count() < max_num_of_apps) {
      list->addItem(
        new QListWidgetItem(QIcon(get_icon(key)), QString::fromStdString(key)));

      app_list.erase(key);
    }

  for (auto& i : app_list)
    if (i.find(search_word) != std::string::npos &&
        list->count() < max_num_of_apps)
      list->addItem(
        new QListWidgetItem(QIcon(get_icon(i)), QString::fromStdString(i)));
}

void
launcher::list_applications(std::string path)
{
  // loop through every source folder
  for (auto file = fs::recursive_directory_iterator(path);
       file != fs::recursive_directory_iterator();
       ++file) {
    const auto name = file->path().filename();
    if (!file->is_directory())
      app_list.insert(name);
  }
}

void
launcher::find_app_icons()
{

  for (auto file = fs::recursive_directory_iterator("/usr/share/icons");
       file != fs::recursive_directory_iterator();
       ++file) {
    app_icons[file->path().filename().stem()] = file->path();
  }
}

const char*
launcher::get_icon(std::string app)
{
  if (app_icons[app] == "\0")
    return default_icon.c_str();

  return app_icons[app].c_str();
}

void
launcher::execute(std::string command)
{
  // Execution mode
  std::string command_to_execute;
  QProcess process(nullptr);
  command_to_execute = command.substr(0, command.find(' ')).c_str();

  if (strcmp(exec_mode, "exec") == 0) {

    process.setProgram("/bin/sh");
    process.setArguments(QStringList{ "-c", command.c_str() });

    if (process.startDetached()) {
      most_used.erase(
        std::remove(most_used.begin(), most_used.end(), command_to_execute),
        most_used.end());
      most_used.insert(most_used.begin(), command_to_execute);
      write_most_used();
      process.close();
      this->close();
    }

  } else if (strcmp(exec_mode, "term") == 0) {
    process.setProgram(default_terminal.c_str());
    QStringList args = QString::fromStdString(command).split(" ");
    args.prepend("-e");
    process.setArguments(args);

    if (fork()) {
      process.start();
      process.waitForFinished(-1);
      QString err = process.readAllStandardError();

      if (err == "") {
        most_used.erase(
          std::remove(most_used.begin(), most_used.end(), command_to_execute),
          most_used.end());
        most_used.insert(most_used.begin(), command_to_execute);
        write_most_used();
      }

    } else
      this->close();
  }
}

void
launcher::keyPressEvent(QKeyEvent* event)
{

  switch (event->key()) {
    case Qt::Key_Return: { // EXECUTE BASH
      std::string command_to_execute;
      if (list->currentRow() < 0) { // If no suggestion is selected
        command_to_execute = input->text().toStdString();
      } else { // if suggestion is selected
        command_to_execute = list->currentItem()->text().toStdString();
      }

      // execution
      try {
        execute(command_to_execute);
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
      this->close();
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
          std::string to_delete = list->currentItem()->text().toStdString();
          most_used.erase(
            std::remove(most_used.begin(), most_used.end(), to_delete),
            most_used.end());

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
}

void
launcher::write_most_used()
{
  std::ofstream file;
  file.open((config_location + "recents.conf").c_str(), std::ios::out);

  for (const auto& key : most_used) {
    file << key.c_str();
    file << "\n";
  }

  file.close();
}

void
launcher::load_most_used()
{
  std::fstream file;
  std::string line;

  file.open((config_location + "recents.conf").c_str(), std::ios::in);
  while (std::getline(file, line)) {
    if (line != "") {
      most_used.push_back(line);
    }
  }
  file.close();
  // std::reverse(most_used.begin(),most_used.end());
}

void
launcher::load_config()
{
  std::fstream file;
  std::string line;

  file.open((config_location + "lofi.conf").c_str(), std::ios::in);

  while (std::getline(file, line)) {
    std::string parm = line.substr(0, line.find('='));
    std::string val = line.substr(line.find('=') + 1, line.size());
    if (configurable.find(parm) != configurable.end()) {
      std::string launcher::*varpointer = configurable.at(parm);
      this->*varpointer = val;
    }
  }

  file.close();
  max_num_of_apps = std::stoi(max_num);
  grid_size = std::stoi(grid_size_stirng);
}
