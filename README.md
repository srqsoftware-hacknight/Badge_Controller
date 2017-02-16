Badge controller and related components

First install Jessie Raspbian 2016-05-27 to SD card.

Mount the SD card and cd into the root directory, then type:

```
touch ssh
```

Unmount the SD card then insert the SD card to the PI and boot it.

SSH into the PI as the "pi" user with default password of "raspberry"

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

SSH back into the PI

Clone projects using git.
```
    git clone https://github.com/srqsoftware-hacknight/Badge_Controller.git
```

Install project (takes up to an hour)
```
    cd Badge_Controller/ControllerInstaller
    sudo ./install.sh root_password pi_password mysql_password wifi_password wifi_ssid controller_hostname webapp_version_number 
```

Replace the parameters to install.sh with your custom values.

NOTE: wifi_password but be between 8 and 64 characters

At this point the raspberry pi reboots, it is setup to act as a badge controller.

When needing to shutdown the PI, SSH in and run:

```
shutdown -h now
```
