RFID_Hardware files and source

First install Jessie Raspbian 2016-05-27 to SD card, and boot into Raspberry Pi 3

Steps to install custom software over Jessie Raspbian 2016-05-27

Update, and install git on raspberry
```
    sudo apt-get update
    sudo apt-get install git
```

Set local for US keyboard instead of UK. (requires reboot)
This is necessary for US keyboards to type '@' and '|' characters.
```
    sudo localectl set-x11-keymap us
    sudo localectl set-keymap us
    reboot
```

Clone projects using git.
```
    cd
    git clone https://github.com/srqsoftware-hacknight/Badge_Web.git
    git clone https://github.com/srqsoftware-hacknight/Badge_Controller.git
```

Install project (takes up to an hour)
```
    sudo bash Badge_Controller/Doormouse/install.sh
```


At this point the raspberry pi reboots, and is set up for stand alone doormouse operation.

The Raspberry Pi will normally run as a read-only filesystem to prevent wear on the sd card.
You can easily set the filesystem to writable with the command ```sudo rw```
When ready to lock it down again, use the command ```sudo ro```

TODO
Add instructions for setting up USB drive to hold MySQL and Jar application.
The Badge_Controller is designed for MySQL database and Jar web application to run off a USB flash drive.
This allows for easy upgrade of software and backup of the database.

