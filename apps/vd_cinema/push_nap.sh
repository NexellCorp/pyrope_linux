#!/bin/bash

NX_DEVICE=
NX_ADB_DEVICES=$(adb devices)
NX_ADB_NETWORK=$(echo $NX_ADB_DEVICES | grep -o "[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}:[0-9]\{1,4\}")

if [[ -n $NX_ADB_NETWORK ]]; then
	NX_DEVICE=$NX_ADB_NETWORK
elif [[ $NX_ADB_DEVICES =~ "NAP" ]]; then
	NX_DEVICE="NAP"
fi

if [[ -z $NX_DEVICE ]]; then
	echo "adb device not connected."
	exit -1;
fi

echo ">> ADB Target Device : $NX_DEVICE"

# Change adb root permission
NX_ADB_ROOT=$(adb -s $NX_DEVICE root)
if [[ $NX_ADB_ROOT != "adbd is already running as root" ]]; then
	echo "change root permission.."
	sleep 4;
fi

# N.AP Start Script
adb -s $NX_DEVICE push script/nap_network /system/bin/
adb -s $NX_DEVICE push script/nap_network.sh /system/bin/
adb -s $NX_DEVICE push script/nap_script.sh /system/bin/

# Digital Cinema Control Library
adb -s $NX_DEVICE push lib/libnxcinema.so /system/lib/

# Test Application
adb -s $NX_DEVICE push bin/ /system/bin/

