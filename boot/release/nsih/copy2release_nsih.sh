#!/bin/bash

SECBOOT_SDMMC=S5P4418_NSIH_V06_sdmmc_800_16x2_1cs_1GB.txt
SECBOOT_USB=S5P4418_NSIH_V06_usb_800_16x2_1cs_1GB.txt

DST_NAME1=nsih_svt
DST_NAME2=nsih_dronel
DST_NAME3=nsih_drone
DST_NAME4=nsih_lepus

cp ../../temporary/nsih/"$SECBOOT_SDMMC" ./"$DST_NAME1"_sdmmc.txt
cp ../../temporary/nsih/"$SECBOOT_SDMMC" ./"$DST_NAME2"_sdmmc.txt
cp ../../temporary/nsih/"$SECBOOT_SDMMC" ./"$DST_NAME3"_sdmmc.txt
cp ../../temporary/nsih/"$SECBOOT_SDMMC" ./"$DST_NAME4"_sdmmc.txt

cp ../../temporary/nsih/"$SECBOOT_USB" ./"$DST_NAME1"_usb.txt
cp ../../temporary/nsih/"$SECBOOT_USB" ./"$DST_NAME2"_usb.txt
cp ../../temporary/nsih/"$SECBOOT_USB" ./"$DST_NAME3"_usb.txt
cp ../../temporary/nsih/"$SECBOOT_USB" ./"$DST_NAME4"_usb.txt
