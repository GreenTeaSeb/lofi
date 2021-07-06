# lofi
An application launcher with less features and customizability!

## content
1. [Shortcuts](#Shortcuts)
2. [Config](#Config)
3. [Customization](#Customization)
4. [Screenshots](#Screenshots)

## Shortcuts
- tab : auto complete first option
- alt : switch between terminal and execution mode
- del : delete recent app

## Config

The default location for the config file is in the home folder at `.config/lofi/`
Currently config only supports

- default terminal for terminal execution
	`default terminal=alacritty`
- default icon
	`default icon=[PATH TO FILE]`
- max number of recents, defaults to 20
	`max recents=20`

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
`

## Screenshots