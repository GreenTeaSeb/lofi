#include "launcher.h"
#include <QPalette>
#include <filesystem>
#include <fstream>
#include <QListWidgetItem>
#include <QList>
#include <QProcess>
#include <algorithm>


namespace fs = std::filesystem;

launcher::launcher(QWidget *parent) :
    QWidget(parent)
{

    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags   | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QPalette palette = this->palette();
    QColor bg_color(backgroundcolor);
    this->resize(720,400);
    this->setMinimumSize(720,400);
    this->setMaximumSize(720,500);


    // loading most used
    load_most_used();

    //pallete stuff
        palette.setColor(QPalette::Window, bg_color);
        this->setAutoFillBackground(true);
        this->setPalette(palette);

    //input
        input->setStyleSheet("color: white;  background:transparent; border: 0px; font-size: 30px");
        mode->setStyleSheet("color: white;  background:rgba(0,0,0,50); border: 0px; font-size: 15px");
        mode->setText(exec_mode);
        mode->setText( " " + mode->text() + ": ");

    //list for all available apps
       list->setStyleSheet("color: white;  background:transparent; border:     0px; font-size: 20px");
       list->setFocusPolicy(Qt::NoFocus);
       list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
       list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
       list->setSpacing(5);

       list_applications(app_locations[0]);
       find_app_icons();
       update_list("");


    //adding everything
        h_layout->addWidget(mode,0);
        h_layout->addWidget(input,10);
        layout->addLayout(h_layout);
        layout->addWidget(list);
        setLayout(layout);
        input->setFocus();



}

void launcher::update_list(std::string search_word){

    list->clear();

    for(const auto &key: most_used)
        if(key.find(search_word) != std::string::npos && list->count() < max_num_of_apps){
               list->addItem(new QListWidgetItem(QIcon(get_icon(key)),QString::fromStdString(key)));

               app_list.erase(key);
        }

    for(auto & i: app_list)
        if( i.find(search_word) != std::string::npos && list->count() < max_num_of_apps)
                list->addItem(new QListWidgetItem(QIcon(get_icon(i)),QString::fromStdString(i)));

}

void launcher::list_applications(std::string   path){
      // loop through every source folder
        for (auto file =  fs::recursive_directory_iterator(path); file != fs::recursive_directory_iterator(); ++file) {
            const auto name = file->path().filename();
            if(!file->is_directory())
                app_list.insert(name);
        }
}

void launcher::find_app_icons(){

        for(auto file = fs::recursive_directory_iterator("/usr/share/icons"); file != fs::recursive_directory_iterator(); ++file){
            app_icons[file->path().filename().stem()] = file->path();
        }

}

const char * launcher::get_icon(std::string app){
    if(app_icons[app] == "\0")
        return default_icon;

    return app_icons[app].c_str();

}

void launcher::execute(std::string command){
    // Execution mode
    std::string command_to_execute;
    QProcess process(nullptr);

    if(strcmp(exec_mode, "exec") ==0){
        command_to_execute =  command;
        process.setProgram("/bin/sh");
        process.setArguments(QStringList{
                                 "-c",
                                 command.c_str()
                             });

    }else if(strcmp(exec_mode, "term") ==0){
        const char* term = getenv("TERM");
        process.setProgram(term);
        process.setArguments(QStringList{

                                 "-e",
                                 command.c_str()
                             });
    }


    if(process.startDetached()){
        most_used.erase(std::remove(most_used.begin(),most_used.end(), command.substr(0,command.find(' ')).c_str()), most_used.end());
        most_used.push_back(command.substr(0,command.find(' ')).c_str());
        write_most_used();
        this->close();
    }else{

    }


    /*
    if(system("firefox")){
        this->close();
    }else{
        //EXECUTED
        most_used.erase(std::remove(most_used.begin(),most_used.end(), command.substr(0,command.find(' ')).c_str()), most_used.end());
        most_used.push_back(command.substr(0,command.find(' ')).c_str());
        write_most_used();
        this->close();
      }

    */
}

void launcher::keyPressEvent(QKeyEvent *event){

    switch (event->key()) {
        case Qt::Key_Return: {// EXECUTE BASH
            std::string command_to_execute;
            if(list->currentRow() < 0){ // If no suggestion is selected
                command_to_execute = input->text().toStdString();
            }else { // if suggestion is selected
                command_to_execute = list->currentItem()->text().toStdString();
            }

            // execution
            try{
                execute(command_to_execute);
              }catch(std::exception e){
                     printf("%s", e.what());
              }
              break;
            }
        case Qt::Key_Backspace: {//DELETE CHARACTER FROM INPUT
            if(input->text().size() > 0)
                input->setText(input->text().chopped(1));
              update_list(input->text().toStdString());
            break;
            }
    case Qt::Key_Up:{
        if(list->currentRow() == 0)
                list->setCurrentRow(list->count()-1);
            else
                list->setCurrentRow(list->currentRow()-1);

        break;}
    case Qt::Key_Down:{
            if(list->currentRow() == list->count()-1)
                list->setCurrentRow(0);
            else
                list->setCurrentRow(list->currentRow()+1);
            break;

            }
        case Qt::Key_Escape:{
           //list->setCurrentRow(-1);
            this->close();
            break;}
    case Qt::Key_Tab:{
            if(list->currentRow() >= 0)
                input->setText(list->currentItem()->text());
            else
                input->setText(list->item(0)->text());
            break;
    }
    case Qt::Key_Alt:{
        if(strcmp(exec_mode,"exec")==0)
            exec_mode = "term";
        else
            exec_mode="exec";

        mode->setText(exec_mode);
        mode->setText( " " + mode->text() + ": ");
        break;
    }
    default:{
            input->setText(input->text()+ event->text());
            update_list(input->text().toStdString().substr(0,input->text().toStdString().find(' ')) );
            break;
    }

    }
}


void launcher::write_most_used(){
    std::ofstream file;
    file.open((config_location + "recents.conf").c_str(), std::ios::out);

    std::reverse(most_used.begin(),most_used.end());

     for(const auto & key: most_used){
         file << key.c_str();

         file << "\n";

     }

     file.close();
}

void launcher::load_most_used(){
    std::fstream file;
    std::string line;

    file.open((config_location + "recents.conf").c_str(),std::ios::in);
    while(std::getline(file,line)){

        if(line != ""){
           most_used.push_back(line);


        }
    }
    file.close();
}

