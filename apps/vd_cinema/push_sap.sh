#!/bin/sh

# Make Directory
adb shell mkdir -p /mnt/mmc/bin
adb shell chmod 755 /mnt/mmc/bin

adb shell mkdir -p /mnt/mmc/lib
adb shell chmod 755 /mnt/mmc/lib

# S.AP Start Script
adb push script/sap_script.sh /mnt/mmc/
adb shell chmod 755 /mnt/mmc/sap_script.sh

# Digital Cinema Control Library.
adb push lib/libnxcinema_linux.so /mnt/mmc/lib/
adb shell chmod 755 /mnt/mmc/lib/*.so

# Secure Link Test Application
adb push apps/sap_slink_client/sap_slink_client /mnt/mmc/bin/
adb shell chmod 755 /mnt/mmc/bin/sap_slink_client

# Ruby Script for Digital Certificate
adb shell mkdir -p /mnt/mmc/bin/cert
adb push script/cert/ /mnt/mmc/bin/cert/
adb shell chmod 755 /mnt/mmc/bin/cert
adb shell chmod 644 /mnt/mmc/bin/cert/*.*
