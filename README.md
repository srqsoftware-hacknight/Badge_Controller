RFID_Hardware files and source

First install Jessie Raspbian 2016-05-27 to SD card, and boot into Raspberry Pi 3

Steps to install custom software over Jessie Raspbian 2016-05-27

Update, and install git on raspberry
```
    sudo apt-get update
    sudo apt-get install git
```

Clone projects using git.
```
    cd
    git clone https://github.com/srqsoftware-hacknight/Badge_Web.git
    git clone https://github.com/srqsoftware-hacknight/Badge_Controller.git
    cd Badge_Controller/Doormouse
```

Install project (takes up to an hour)
```
    sudo bash install.sh
```

At this point the raspberry pi reboots, and is set up for stand alone doormouse operation.

