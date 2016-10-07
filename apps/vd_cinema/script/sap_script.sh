#!/bin/sh

# S.AP Network Configuration
ifconfig eth0 192.168.254.243 netmask 255.255.255.240 up
#ifconfig eth0 192.168.1.222 netmask 255.255.255.0 up
#route add default gw 192.168.1.254 eth0

# Start Application
export PATH=$PATH:/mnt/mmc/bin
export LD_LIBRARY_PATH=/mnt/mmc/lib

cd /mnt/mmc/bin/cert
ruby make-dc-certificate-chain.rb

cd /mnt/mmc/bin
./sap_slink_client &
