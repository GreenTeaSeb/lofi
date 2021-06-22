#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QListWidget>
#include <vector>
#include <unordered_set>
#include <map>
#include <stdlib.h>


class launcher : public QWidget
{
    Q_OBJECT

    QLabel *input = new QLabel(this);
    QLabel *mode = new QLabel(this);
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    QListWidget *list = new QListWidget(this);
    const char* exec_mode = "exec";

    int max_num_of_apps = 20;
    std::string default_icon = "/usr/share/icons/Papirus/24x24/apps/bash.svg";
    std::string config_location =  std::string(getenv("HOME")) + "/.config/lofi/";
    std::string default_terminal = (getenv("TERM")) ? getenv("TERM") : "" ;
    std::string backgroundcolor = "#282a36";



    std::vector<std::string> app_locations = {"/usr/bin"};

    std::unordered_set<std::string> app_list;
    std::vector<std::string> most_used;
    std::map<std::string,std::string> app_icons;



public:
    explicit launcher(QWidget *parent = nullptr);

    std::map<std::string, std::string launcher::*> configurable = {
        {"default icon",  &launcher::default_icon},
        {"background color",  &launcher::backgroundcolor},
        {"default terminal",  &launcher::default_terminal},
        {"config location",  &launcher::config_location},
    };


protected:

    void keyPressEvent(QKeyEvent *event) override;

    void list_applications(std::string path);

    void update_list(std::string search_word);

    void find_app_icons();

    void write_most_used();

    void load_most_used();
    void load_config();

    void execute(std::string command);

    const char* get_icon(std::string);


    // configurable
signals:

};

#endif // LAUNCHER_H
