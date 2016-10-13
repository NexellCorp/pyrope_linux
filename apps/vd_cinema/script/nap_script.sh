#!/system/bin/sh

# N.AP Network Configuration
ifconfig eth0 192.168.10.222 netmask 255.255.255.0
ndc network create 100 
ndc network interface add 100 eth0
ndc network route add 100 eth0 192.168.10.0/24
ndc network route add 100 eth0 0.0.0.0/0 192.168.10.1
ndc resolver setnetdns 100 localdomain 8.8.8.8
ndc network default set 100

# NTP daemon
busybox ntpd -p 115.90.134.38 -l

# Start Application
/system/bin/nap_server &
