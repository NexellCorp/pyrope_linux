#!/bin/sh

# Make Directory
adb -s SAP shell mkdir -p /mnt/mmc/bin
adb -s SAP shell chmod 755 /mnt/mmc/bin

adb -s SAP shell mkdir -p /mnt/mmc/lib
adb -s SAP shell chmod 755 /mnt/mmc/lib

# S.AP Start Script
adb -s SAP push script/sap_script.sh /mnt/mmc/
adb -s SAP shell chmod 755 /mnt/mmc/sap_script.sh

# Digital Cinema Control Library.
adb -s SAP push lib/libnxcinema_linux.so /mnt/mmc/lib/
adb -s SAP shell chmod 755 /mnt/mmc/lib/*.so

# Secure Link Test Application
adb -s SAP push apps/sap_slink_client/sap_slink_client /mnt/mmc/bin/
adb -s SAP shell chmod 755 /mnt/mmc/bin/sap_slink_client

# Ruby Script for Digital Certificate
adb -s SAP shell mkdir -p /mnt/mmc/bin/cert
adb -s SAP push script/cert/ /mnt/mmc/bin/cert/
adb -s SAP shell chmod 755 /mnt/mmc/bin/cert
adb -s SAP shell chmod 644 /mnt/mmc/bin/cert/*.*
