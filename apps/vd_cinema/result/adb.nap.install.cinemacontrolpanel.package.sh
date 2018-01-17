#!/bin/bash
adb -s NAP push ./image/libnxcinema.so /system/lib/
adb -s NAP push ./image/nap_server /system/bin/
adb -s NAP install -r ./image/CinemaControlPanel.apk
adb -s NAP shell sync

