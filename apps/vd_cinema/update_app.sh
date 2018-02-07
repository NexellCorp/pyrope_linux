#!/bin/sh
NX_DIR_WORKING=$(pwd)
NX_DIR_SCRIPT=$(cd "$(dirname "$0")" && pwd)
NX_DIR_ANDROID=$NX_DIR_SCRIPT/../../../../..

NX_DIR_APK=$NX_DIR_SCRIPT/apps/CinemaSolution/CinemaControlPanel/build/outputs/apk
NX_SRC_APK=$NX_DIR_APK/CinemaControlPanel-debug.apk

NX_DIR_TARGET=$NX_DIR_ANDROID/vendor/nexell/apps/CinemaControlPanel
NX_DIR_TARGET_APK=$NX_DIR_TARGET/app
NX_DIR_TARGET_LIB=$NX_DIR_TARGET/lib/arm

# Update library
unzip -jo $NX_SRC_APK "*.so" -d $NX_DIR_TARGET_LIB
chmod 744 $NX_DIR_TARGET_LIB/*.so

# Update APK
cp -av $NX_SRC_APK $NX_DIR_TARGET_APK/$(basename $NX_SRC_APK | sed "s/-debug//")
