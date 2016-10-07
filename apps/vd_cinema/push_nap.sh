#!/bin/sh

# N.AP Start Script
adb push script/nap_script.sh /system/bin/

# Digital Cinema Control Library.
adb push lib/libnxcinema.so /system/lib/

# Test Application
adb push bin/ /system/bin/
