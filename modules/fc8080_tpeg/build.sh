#!/bin/sh
make ARCH=arm clean
make ARCH=arm -j4
# cp -v fc8080_tpeg.ko ../../../common/fs/buildroot/out/rootfs/
