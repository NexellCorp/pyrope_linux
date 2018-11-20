#!/bin/bash
if [ ! -d $1 ]
then
	echo "Usage: $0 [RESULT DIRECTORY]"
	exit -1;
fi

NX_DIR_WORKING=$(pwd)
NX_DIR_SCRIPT=$(cd "$(dirname "$0")" && pwd)
NX_DIR_TARGET=$NX_DIR_SCRIPT/release/NxUsbBuner/nap
NX_DATE="$(date +%Y%m%d)"

NX_DIR_RESULT=$1
NX_PACKAGE=(
	"$NX_DIR_RESULT/u-boot.bin"
	"$NX_DIR_RESULT/boot.img"
	"$NX_DIR_RESULT/system.img"
	"$NX_DIR_RESULT/cache.img"
	"$NX_DIR_RESULT/userdata.img"
)

for package in "${NX_PACKAGE[@]}"; do
	package_name=$(basename $package)
	result=$NX_DIR_TARGET/$package_name
	if ! test -e $package; then
		continue
	fi

	cmp -s $package $result
	if [ $? -ne 0 ]; then
		echo ">>> Update $package_name"
		cp -av $package $result
	fi
done

echo ">>> Package TarBall."
cd $NX_DIR_SCRIPT
tar cvzf VDCinemaRelease_$NX_DATE.tar.gz release
cd $NX_DIR_WORKING

echo ">>> Package Zip."
cd $NX_DIR_SCRIPT/release
zip -r $NX_DIR_SCRIPT/VDCinemaRelease_$NX_DATE.zip *
cd $NX_DIR_WORKING
