#!/bin/bash
adb -s NAP shell rm /storage/sdcard0/SAMSUNG/TCON_USB/*.txt
adb -s NAP shell rm /storage/sdcard0/SAMSUNG/TCON_USB/LUT/*.txt
adb -s NAP shell rm /storage/sdcard0/SAMSUNG/TCON_BEHAVIOR/*.txt
adb -s NAP shell sync
