adb shell mkdir -p /mnt/mmc/bin
adb shell mkdir -p /mnt/mmc/lib

adb push script/run_adb.sh /mnt/mmc/

adb push lib/libnxcinema_linux.so /mnt/mmc/lib/
adb push apps/sap_marriage/sap_marriage /mnt/mmc/bin/

adb shell mkdir -p /mnt/mmc/bin/ruby
adb push apps/sap_marriage/ruby/ /mnt/mmc/bin/ruby/
