#!/bin/bash

SEC4418_NAME=S5P4418_2ndboot_DDR3_V061
SEC4330_NAME=NXP4330_2ndboot_DDR3_V061

DST4418_NAME1=2ndboot_svt
DST4418_NAME2=2ndboot_dronel
DST4418_NAME3=2ndboot_drone

DST4330_NAME1=2ndboot_lepus

echo "$SECBOOT_NAME"

cp ../../temporary/2ndboot/"$SEC4330_NAME"_SDMMC.bin ./"$DST4330_NAME1"_sdmmc.bin
#cp ../../temporary/2ndboot/"$SEC4330_NAME"_USB.bin   ./"$DST4330_NAME1"_usb.bin

cp ../../temporary/2ndboot/"$SEC4418_NAME"_SDMMC.bin ./"$DST4418_NAME1"_sdmmc.bin
cp ../../temporary/2ndboot/"$SEC4418_NAME"_SDMMC.bin ./"$DST4418_NAME2"_sdmmc.bin
cp ../../temporary/2ndboot/"$SEC4418_NAME"_SDMMC.bin ./"$DST4418_NAME3"_sdmmc.bin

cp ../../temporary/2ndboot/"$SEC4418_NAME"_USB.bin ./"$DST4418_NAME1"_usb.bin
cp ../../temporary/2ndboot/"$SEC4418_NAME"_USB.bin ./"$DST4418_NAME2"_usb.bin
cp ../../temporary/2ndboot/"$SEC4418_NAME"_USB.bin ./"$DST4418_NAME3"_usb.bin

cp ../../temporary/2ndboot/"$SEC4418_NAME"_SDFS.bin ./"$DST4418_NAME1"_sdfs.bin
cp ../../temporary/2ndboot/"$SEC4418_NAME"_SPI.bin  ./"$DST4418_NAME1"_spi.bin
cp ../../temporary/2ndboot/"$SEC4418_NAME"_NAND.bin ./"$DST4418_NAME1"_nand.bin
