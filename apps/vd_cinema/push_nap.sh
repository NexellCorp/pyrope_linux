#!/bin/sh

# Change adb root permission
adb -s NAP root

# N.AP Start Script
adb -s NAP push script/nap_network /system/bin/
adb -s NAP push script/nap_network.sh /system/bin/
adb -s NAP push script/nap_script.sh /system/bin/

# Digital Cinema Control Library
adb -s NAP push lib/libnxcinema.so /system/lib/

# Test Application
adb -s NAP push bin/ /system/bin/
