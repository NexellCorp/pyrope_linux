#!/system/bin/sh

# N.AP Network Configuration
/system/bin/nap_network.sh start

# NTP daemon
busybox ntpd -p 216.239.35.0 -p 216.239.35.4 -p 216.239.35.8 -p 216.239.35.12 -l

# Start Application
/system/bin/nap_server &
