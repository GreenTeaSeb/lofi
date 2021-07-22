#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMimeType>
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

  enum class modes
  {
    exec,
    term,
    file
  };
  modes exec_mode = modes::exec;

  int max_num_of_apps = 20;
  int grid_size = 128;

  std::string default_icon = "";
  std::string config_location = std::string(getenv("HOME")) + "/.config/lofi/";
  std::string default_terminal = (getenv("TERM")) ? getenv("TERM") : "";
  std::string max_num = "20";
  std::string list_layout = "list";
  std::string grid_size_stirng = "128";

  QString dir_path = {};
  QString stylesheet = {};

  std::vector<std::string> app_locations = { "/usr/share/applications" };

  QStringList app_list;
  QStringList most_used;

  // .desktop data
  static constexpr int NAME_ROLE = Qt::UserRole + 0;
  static constexpr int EXEC_ROLE = Qt::UserRole + 1;
  static constexpr int ICON_ROLE = Qt::UserRole + 2;
  static constexpr int SEARCHABLE_ROLE = Qt::UserRole + 3;
  static constexpr int TYPE_ROLE = Qt::UserRole + 4;
  static constexpr int PATH_ROLE = Qt::UserRole + 5;

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
  void update_list(std::string search_word);

  void list_files(QString path, QString search_word);

  void add_item(QString path, QString search_word);
  void initalize_list();
  void set_exec_mode();

  void write_most_used();

  void load_most_used();
  void load_config();
  void load_stylesheet();

  void exit();

  void execute();
  void start_process(QString command);
  void parse_arguements();

  QIcon get_icon(QString name);
  QIcon get_icon_file(QString name);

  // configurable
signals:
public slots:
  void item_double_click(QListWidgetItem* item);
};

#endif // LAUNCHER_H
