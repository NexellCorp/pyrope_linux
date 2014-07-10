#!/bin/sh
make ARCH=arm clean
make ARCH=arm modules -j4
cp wlan.ko ../../../../../hardware/nexell/pyrope/prebuilt/modules/
