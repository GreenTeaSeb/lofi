# lofi
![screenshot](https://raw.githubusercontent.com/GreenTeaSeb/lofi/senpai/screenshots/example.png)  
An application launcher made with Qt with less features and customizability!


Todo:  
- pinned applications

If you use a WM, you will need to make this app launch in floating mode manually  
- Sway `for_window [app_id="lofi"] floating enable`  

## Modes
Term: whatever you type in will be launched in your default terminal  
Exec: Launched the process directly, anything after the name of the application will be used as arguements, so be careful of spaces.
File: file browser, opens up files with the default application that is set for that type   

## Usage
launch with flag -r to use it as a file/app launcher, without the flag it will display options from stdin and output them to stdout  

## content
1. [Shortcuts](#Shortcuts)
2. [Config](#Config)
3. [Customization](#Customization)
4. [Screenshots](#Screenshots)

## Shortcuts
- tab : auto complete first option  
- alt : switch between terminal and execution mode  
- del : delete selected recent app from list  
- enter : run selected app in list or if nothing is selected, the command in the input bar
- esc : exit launcher  
- ctrl+r : reload the cache and program

## Config

The default location for the config file is in the home folder at `.config/lofi/`
Currently config only supports

- default terminal for terminal execution, default value needs the HOME env to be set.
	`default terminal = alacritty`
- default icon for when an icon is not found
	`default icon = [PATH TO FILE]`
- max number of recents, defaults to 20
	`max recents = 20`
- list layout to set the type of list you want
      `layout = grid` sets the layout of list to a grid
- grid size , if the layout is set to the grid, this will control how big the grid is, defaults to 128
      `grid size = 128`   
      NOTE: this is seperate from the stylesheet and if the stylesheet icon-size is too big, it will clip through other icons, this is why you set this   

- check locations, paths to check for files, by default it's only "/usr/bin"  
      `check locations = /usr/bin , /home/seb/.local`  
      Adding more locations will cause a bigger cache to be created and slower perfomance when searching depending on the amount of files in the location



## Customization
To customize how the app launcher looks simply  edit the `style.qss` file at the config folder.
It uses QTs stylesheet, which is mostly like CSS, so you can use it easily if you know CSS  
In case you don't know how to use it, [here are the official examples](https://doc.qt.io/qt-5/stylesheet-syntax.html)

Here is the default style:

	#main{
		background: rgb(50, 50, 50);
	}

	#list{
		color: white;
		background:transparent;
		border: 0px;
		font-size: 20px
	}

	#input{
		color: white;
		background:transparent;
		border: 0px;
		font-size: 30px;
	}

	#mode{
		color: white;
		background:rgba(0,0,0,50);
		border: 0px;
		font-size: 15px;
	}

Here's an example of a customized style  

      #main{
            background: transparent;
            border-image:url(/home/seb/Pictures/backgrounds/back.png) 0 0 0 0 stretch stretch;
            border-radius: 20px;
            
      }     
      
      
      #list{      
            color: white;
            background:rgba(0, 0, 0, 100);
            border-radius: 10px;
            padding: 10px;
            font-size: 20px;
            font-display: none;
            icon-size: 30px;
      }
      
      
      #list::item{
            selection-background-color: rgba(0, 0, 0, 0.253) ;  
            selection-color: rgb(255, 255, 255);
                  
      }
      
      #list::item:selected{
            font-size: large;
      }
      
      #input, #mode{ 
            color: white;
            background:rgba(0, 0, 0, 100);
            border-radius: 10px;
            padding: 10px;
            font-size: 20px;
      }

## Screenshots
- Default theme  
![screenshot](https://raw.githubusercontent.com/GreenTeaSeb/lofi/senpai/screenshots/defaultstyle.png)  
- Customized style  
![screenshot](https://raw.githubusercontent.com/GreenTeaSeb/lofi/senpai/screenshots/customstyle.png)
- Config with layout=grid  
![screenshot](https://raw.githubusercontent.com/GreenTeaSeb/lofi/senpai/screenshots/customstyle_grid.png)