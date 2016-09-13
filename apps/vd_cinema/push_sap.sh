adb shell mkdir -p /mnt/mmc/bin
adb shell mkdir -p /mnt/mmc/lib

adb push script/run_adb.sh /mnt/mmc/

# Digital Cinema Control Library.
adb push lib/libnxcinema_linux.so /usr/lib/
adb push lib/libnxcinema_linux.so /mnt/mmc/lib/

# Secure Link Test Application
adb push apps/sap_slink_client/sap_slink_client /mnt/mmc/bin/

# Ruby Script for Digital Certificate
adb shell mkdir -p /mnt/mmc/bin/ruby
adb push script/ruby/ /mnt/mmc/bin/ruby/
