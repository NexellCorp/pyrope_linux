.\bin\adb.exe -s NAP push .\image\libnxcinema.so /system/lib/
.\bin\adb.exe -s NAP push .\image\nap_server /system/bin/
.\bin\adb.exe -s NAP shell sync
pause