#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

binpath=/usr/local/bin
sysdpath=/etc/systemd/system
program=ir_ook_sender.py

echo "Set your MQTT connection"
echo "Broker address (localhost):"
read mqttbroker
mqttbroker=${mqttbroker:-localhost}
echo "Port (1883)"
read mqttport
mqttport=${mqttport:-1883}
echo "Username (None)"
read mqttuser
echo "Password (None)"
read -s mqttpass
echo "You entered $mqttbroker, $mqttport, $mqttuser, $mqttpass"

cp -v $program tmp.py
r1="s/\('MQTT_BROKER' *\:\)[^,]\+/\1 '"$mqttbroker"'/"
r2="s/\('MQTT_PORT' *\:\)[^,]\+/\1 $mqttport/"
r3="s/\('MQTT_USERNAME' *\:\)[^,]\+/\1 '"$mqttuser"'/"
r4="s/\('MQTT_PASSWORD' *\:\)[^,]\+/\1 '"$mqttpass"'/"

sed -i "$r1" tmp.py
sed -i "$r2" tmp.py
[[ ! -z "$mqttuser" ]] && sed -i "$r3" tmp.py
[[ ! -z "$mqttpass" ]] && sed -i "$r4" tmp.py


#install configs and systemd
cp -v *.service $sysdpath
cp -v tmp.py "$binpath/$program"
rm tmp.py

echo "I will try to fulfill requirements for python, but you may have to manuall pip3 install modules in requirements.txt"
/usr/bin/pip3 install -r requirements.txt
