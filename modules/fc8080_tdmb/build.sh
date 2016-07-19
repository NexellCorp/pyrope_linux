#!/bin/sh
make ARCH=arm clean
make ARCH=arm -j4
# cp -v fc8080_tdmb.ko ../../../common/fs/buildroot/out/rootfs/
