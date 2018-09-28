#!/bin/bash
NX_DIR_WORKING=$(pwd)
NX_DIR_SCRIPT=$(cd "$(dirname "$0")" && pwd)
NX_DIR_TARGET=$NX_DIR_SCRIPT/package/image
NX_DATE="$(date +%Y%m%d)_$(date +%H%M%S)"

NX_DIR_LIB=$NX_DIR_SCRIPT/lib
NX_DIR_BIN=$NX_DIR_SCRIPT/bin
NX_DIR_APK=$NX_DIR_SCRIPT/apps/CinemaSolution/CinemaControlPanel/build/outputs/apk

NX_PACKAGE=(
	"$NX_DIR_LIB/libnxcinema.so"
	"$NX_DIR_BIN/nap_server"
	"$NX_DIR_APK/CinemaControlPanel-debug.apk"
)

mkdir -p $NX_DIR_TARGET

for package in "${NX_PACKAGE[@]}"; do
	package_name=$(basename $package | sed "s/-debug//")
	result=$NX_DIR_TARGET/$package_name

	cmp -s $package $result
	if [ $? -ne 0 ]; then
		echo ">>> Update $package_name"
		cp -av $package $result
	fi
done

cd $NX_DIR_SCRIPT
tar cvzf package_$NX_DATE.tar.gz package

cd $NX_DIR_WORKING
