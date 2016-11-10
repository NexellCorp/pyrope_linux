#!/system/bin/sh

# N.AP Network Configuration
/system/bin/nap_network.sh start

# NTP daemon
busybox ntpd -p 115.90.134.38 -l

# Start Application
/system/bin/nap_server &
