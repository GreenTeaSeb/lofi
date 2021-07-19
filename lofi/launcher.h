#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <map>
#include <stdlib.h>

class launcher : public QWidget
{
  Q_OBJECT

  QLabel* input = new QLabel(this);
  QLabel* mode = new QLabel(this);
  QVBoxLayout* layout = new QVBoxLayout();
  QHBoxLayout* h_layout = new QHBoxLayout();
  QListWidget* list = new QListWidget(this);

  const char* exec_mode = "exec";

  int max_num_of_apps = 20;
  int grid_size = 128;

  std::string default_icon = "";
  std::string config_location = std::string(getenv("HOME")) + "/.config/lofi/";
  std::string default_terminal = (getenv("TERM")) ? getenv("TERM") : "";
  std::string max_num = "20";
  std::string list_layout = "list";
  std::string grid_size_stirng = "128";

  QString stylesheet = {};

  std::vector<std::string> app_locations = { "/usr/bin" };

  QStringList app_list;
  QStringList most_used;

public:
  bool app_launcher = false;
  explicit launcher(QWidget* parent = nullptr);

  std::map<std::string, std::string launcher::*> configurable = {
    { "default icon", &launcher::default_icon },
    { "default terminal", &launcher::default_terminal },
    { "max recents", &launcher::max_num },
    { "layout", &launcher::list_layout },
    { "grid size", &launcher::grid_size_stirng }
  };

protected:
  void keyPressEvent(QKeyEvent* event) override;

  void list_applications();
  void load_list();

  void initalize_list();
  void update_list(std::string search_word);

  void write_most_used();

  void load_most_used();
  void load_config();
  void load_stylesheet();

  void exit();

  void execute();
  void parse_arguements();

  std::string to_upper(std::string input);
  QIcon get_icon(std::string);

  // configurable
signals:
};

#endif // LAUNCHER_H
