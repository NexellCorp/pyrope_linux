#!/system/bin/sh

# N.AP Network Configuration
ifconfig eth0 192.168.10.222 netmask 255.255.255.0
ndc network create 100 
ndc network interface add 100 eth0
ndc network route add 100 eth0 192.168.10.0/24
ndc network default set 100

# Start Application
/system/bin/nap_server &
