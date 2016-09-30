#!/bin/bash

########################################################################################
#  Convert a Raspian Lite install to the Doormouse key entry server                    #
#  Start with 2016-05-27-raspbian-jessie-lite, run install.sh (this) as root           #
#                                                                                      #
#  Set passwords, wifi etc below:                                                      #
ROOTPASS=alicechasingrabbits
MYSQLPASS=alicechasingrabbits
WIFIPASS=alicechasingrabbits
WIFISSID=TheDoormouse
#                                                                                      #
# Place your public ssh keys in ./files/public_keys for root access via ssh            #
########################################################################################

base_system(){
 echo "###############  Installing software from repositories  ###############"
 apt-get --force-yes -y install vim
 apt-get --force-yes -y install gawk
 apt-get --force-yes -y install ntpdate
 apt-get --force-yes -y install usbutils
 apt-get --force-yes -y install apache2
  echo 'ExecStartPre=/bin/mkdir /var/log/apache2' >> /lib/systemd/system/apache2.service.d/forking.conf
 apt-get --force-yes -y install dnsmasq
 apt-get --force-yes -y install hostapd
 apt-get --force-yes -y install tcpdump
 cp ./files/startdir /sbin/
  chmod 755 /sbin/startdir
 #mysql server
  debconf-set-selections <<< "mysql-server mysql-server/root_password password $MYSQLPASS"
  debconf-set-selections <<< "mysql-server mysql-server/root_password_again password $MYSQLPASS"
  apt-get --force-yes -y install mysql-server
echo ...Done
}

set_wifi(){
echo "###############  Setting up Wifi SSID and Password  ###############"
  sed -i 's/^ssid=.*/ssid='$WIFISSID'/' /etc/hostapd/hostapd.conf
  sed -i 's/^wpa_passphrase=.*/wpa_passphrase='$WIFIPASS'/' /etc/hostapd/hostapd.conf
echo ...Done
}

set_root_pass(){
echo "###############  Setting Root password  ###############"
  echo -e "$ROOTPASS\n$ROOTPASS" | passwd root
echo ...Done
}

set_watchdog(){
echo "###############  Setting up System Watchdog  ###############"
  modprobe bcm2835_wdt
  apt-get --force-yes -y install watchdog
  sed -i 's/^#watchdog-device.*/watchdog-device = \/dev\/watchdog/' /etc/watchdog.conf
  sed -i 's/^#max-load-1 .*/max-load-1 = 24/' /etc/watchdog.conf
  echo 'WantedBy=multi-user.target' >> /lib/systemd/system/watchdog.service
  echo 'kernel.panic = 10' >> /etc/sysctl.conf 
  systemctl enable watchdog
echo ...Done
}

set_locale(){
echo "###############  Setting Locale  ###############"
# apt-get --force-yes -y install locales
# locale-gen en_US en_US.UTF-8
# localedef -v -c -i en_US -f UTF-8 en_US.UTF-8 
cp /usr/share/zoneinfo/America/New_York /etc/localtime
echo ...Done
}

pretty_command_line(){
 echo "############### Making the command line more friendly ##############"
 sed -i "s/# export LS_OPTIONS='--color=auto'/export LS_OPTIONS='--color=auto'/g" ~/.bashrc
 sed -i 's/# eval "`dircolors`"/eval "`dircolors`"/g' ~/.bashrc
 sed -i 's/"syntax on/syntax on/g' /etc/vim/vimrc
echo ...Done
}

#############################################################################
public_keys(){
 echo "###############  Setting up ssh keys  ###############"
 mkdir /root/.ssh
 cat /dev/null >/root/.ssh/authorized_keys
 cat files/public_keys/*.pub >>/root/.ssh/authorized_keys
 chmod 600 /root/.ssh/authorized_keys
echo ...Done
}

config(){
 echo "###############  Setting up system config  ###############"
 cp ./files/motd /etc/motd
 echo Doormouse > /etc/hostname
 sed -i 's/raspberrypi/Doormouse/g' /etc/hosts
 #Wifi
  cp ./files/config/dhcpcd.conf /etc/dhcpcd.conf
  cp ./files/config/hostapd.conf /etc/hostapd/hostapd.conf
   sed -i 's/#DAEMON_CONF=""/DAEMON_CONF="\/etc\/hostapd\/hostapd.conf"/g' /etc/default/hostapd
  cp ./files/config/dnsmasq.conf /etc/dnsmasq.conf
  cp ./files/config/interfaces /etc/network/interfaces
echo ...Done
}

read_only(){
 echo "###############  Setting Main Partition for Read-Only  ###############"
 apt-get --force-yes -y install busybox-syslogd
 dpkg --purge rsyslog
 sed -i '/^dwc_otg.lpm_enable/ s/$/ fastboot noswap ro/' /boot/cmdline.txt 
 rm -rf /var/lib/dhcp/ /var/spool /var/lock
  ln -s /tmp /var/lib/dhcp 
  ln -s /tmp /var/spool 
  ln -s /tmp /var/lock
 sed -i 's/PIDFile=\/run\/dhcpcd.pid/PIDFile=\/var\/run\/dhcpcd.pid/' /etc/systemd/system/dhcpcd5
 rm /var/lib/systemd/random-seed
 ln -s /tmp/random-seed /var/lib/systemd/random-seed
 echo 'ExecStartPre=/bin/echo "" >/tmp/random-seed' >> /lib/systemd/system/systemd-random-seed.service
 sed -i 's/driftfile \/var\/lib\/ntp\/ntp.drift/driftfile \/var\/tmp\/ntp.drift/' /etc/ntp.conf
 insserv -r bootlogs
 insserv -r console-setup
 cp ./files/config/fstab /etc/fstab
 cp ./files/ro /sbin/
 cp ./files/rw /sbin/
 swapoff /var/swap
 rm -f /var/swap
 systemctl daemon-reload
echo ...Done
}

apt-get update
####################### Install Groups ######################### 
read_only
base_system
set_locale
pretty_command_line
public_keys
config
set_wifi
set_root_pass
set_watchdog
################################################################
echo ''
echo 'DoorMouse Install Complete. Rebooting...'
sleep 2
reboot
