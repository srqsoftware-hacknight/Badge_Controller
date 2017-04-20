#!/bin/bash

set -e

HOME_DIR=/home/pi
ROOTPASS=""
PIPASSWORD=""
MYSQLPASS=""
WIFIPASS=""
WIFISSID=""
HOSTNAME=""
WEBAPP_VER=""

DONE_MSG="...Done"

log() {
  echo "$@"

  if [ ! -f "install.log" ]; then
    echo "# Starting install" > install.log
  fi

  echo "# $@" >> install.log
}

read_only() {
  log "Setting Main Partition for Read-Only"

  apt-get --force-yes -y install busybox-syslogd
  
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

  systemctl daemon-reload

  log $DONE_MSG
}

base_system() {
  log "Installing software from repositories"

  apt-get --force-yes -y install vim
  apt-get --force-yes -y install gawk
  apt-get --force-yes -y install ntpdate
  apt-get --force-yes -y install usbutils
  apt-get --force-yes -y install oracle-java8-jdk
  apt-get --force-yes -y install dnsmasq
  apt-get --force-yes -y install hostapd
  apt-get --force-yes -y install tcpdump

  debconf-set-selections <<< "mysql-server mysql-server/root_password password $MYSQLPASS"
  debconf-set-selections <<< "mysql-server mysql-server/root_password_again password $MYSQLPASS"

  apt-get --force-yes -y install mysql-server

  service mysql start
  update-rc.d mysql defaults

  mysql -u root -p$MYSQLPASS -e 'create database badge;'

  log $DONE_MSG
}

set_locale() {
  log "Setting Locale"

  cp /usr/share/zoneinfo/America/New_York /etc/localtime

  log $DONE_MSG
}

pretty_command_line() {
  log "Making the command line more friendly"

  sed -i "s/# export LS_OPTIONS='--color=auto'/export LS_OPTIONS='--color=auto'/g" ~/.bashrc
  sed -i 's/# eval "`dircolors`"/eval "`dircolors`"/g' ~/.bashrc
  sed -i 's/"syntax on/syntax on/g' /etc/vim/vimrc

  log $DONE_MSG
}

config() {
  log "Setting up system config"

  cp ./files/motd /etc/motd
  echo $HOSTNAME > /etc/hostname
  sed -i "s/raspberrypi/$HOSTNAME/g" /etc/hosts

  cp ./files/config/dhcpcd.conf /etc/dhcpcd.conf
  cp ./files/config/hostapd.conf /etc/hostapd/hostapd.conf
  sed -i 's/#DAEMON_CONF=""/DAEMON_CONF="\/etc\/hostapd\/hostapd.conf"/g' /etc/default/hostapd
  cp ./files/config/dnsmasq.conf /etc/dnsmasq.conf
  cp ./files/config/interfaces /etc/network/interfaces
  cp ./files/config/rc.local /etc/
  chmod 755 /etc/rc.local

  log $DONE_MSG
}

set_wifi() {
  log "Setting up Wifi SSID and Password"

  sed -i 's/^ssid=.*/ssid='$WIFISSID'/' /etc/hostapd/hostapd.conf
  sed -i 's/^wpa_passphrase=.*/wpa_passphrase='$WIFIPASS'/' /etc/hostapd/hostapd.conf

  log $DONE_MSG
}

set_passwords() {
  log "Setting Root password"

  echo -e "$ROOTPASS\n$ROOTPASS" | passwd root

  log "Setting Pi password"

  echo -e "$PIPASSWORD\n$PIPASSWORD" | passwd pi
  
  log $DONE_MSG
}

set_watchdog() {
  log "Setting up System Watchdog"

  modprobe bcm2835_wdt

  apt-get --force-yes -y install watchdog

  sed -i 's/^#watchdog-device.*/watchdog-device = \/dev\/watchdog/' /etc/watchdog.conf
  sed -i 's/^#max-load-1 .*/max-load-1 = 24/' /etc/watchdog.conf

  echo 'WantedBy=multi-user.target' >> /lib/systemd/system/watchdog.service
  echo 'kernel.panic = 10' >> /etc/sysctl.conf 

  systemctl enable watchdog

  log $DONE_MSG
}

set_args() {
  if [ "$#" -ne 7 ]; then
    display_help
    exit 1
  fi

  ROOTPASS="$1"
  PIPASSWORD="$2"
  MYSQLPASS="$3"
  WIFIPASS="$4"
  WIFISSID="$5"
  HOSTNAME="$6"
  WEBAPP_VER="$7"
}

display_help() {
  echo "Invalid number of parameters."
  echo "./$(basename $0) root_password pi_password mysql_password wifi_password wifi_ssid controller_hostname webapp_version_number"
}

setup_webapp() {
  cat <<EOF > $HOME_DIR/application.properties
server.port=80

spring.datasource.url=jdbc:mysql://localhost/badge
spring.datasource.username=root
spring.datasource.password=$MYSQLPASS
spring.datasource.driver-class-name=com.mysql.jdbc.Driver
spring.datasource.platform=mysql

EOF

  curl --silent -L -o $HOME_DIR/badge-web.war https://github.com/srqsoftware-hacknight/Badge_Web/releases/download/$WEBAPP_VER/badge-web-$WEBAPP_VER.war
}

setup_wiringpi() {
  git clone git://git.drogon.net/wiringPi
  cd wiringPi
  git checkout -b 2.44 2.44
  ./build
}

install() {
  set_args "$@"

  pushd $(cd `dirname "${BASH_SOURCE[0]}"` && pwd) >> /dev/null

  apt-get update

  read_only
  base_system
  set_locale
  pretty_command_line
  config
  set_wifi
  set_passwords
  set_watchdog
  setup_webapp
  setup_wiringpi

  log ""
  log "Install Complete. Rebooting..."

  reboot	
}

install "$@"
