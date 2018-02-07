#!/bin/bash
adb -s NAP push ./image/libnxcinema.so /system/lib/
adb -s NAP shell sync
