Badge controller and related components

First install Jessie Raspbian 2016-05-27 to SD card, and boot into Raspberry Pi 3

Steps to install custom software over Jessie Raspbian 2016-05-27

Update, and install git on raspberry
```
    sudo apt-get update
    sudo apt-get -y install git
```

Set local for US keyboard instead of UK. (requires reboot)
This is necessary for US keyboards to type '@' and '|' characters.
```
    sudo localectl set-x11-keymap us
    sudo localectl set-keymap us
    sudo reboot
```

Clone projects using git.
```
    cd
    git clone https://github.com/srqsoftware-hacknight/Badge_Controller.git
```

Install project (takes up to an hour)
```
    cd Badge_Controller/ControllerInstaller
    sudo ./install.sh root_password pi_password mysql_password wifi_password wifi_ssid controller_hostname
```

Replace the parameters to install.sh with your custom values.

At this point the raspberry pi reboots, it is setup to act as a badge controller.

The Raspberry Pi will normally run as a read-only filesystem to prevent wear on the sd card.
You can easily set the filesystem to writable with the command ```sudo rw```
When ready to lock it down again, use the command ```sudo ro```

