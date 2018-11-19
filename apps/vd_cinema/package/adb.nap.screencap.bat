.\bin\adb.exe -s NAP shell screencap -p /system/bin/capture.png
.\bin\adb.exe -s NAP pull /system/bin/capture.png .\result\capture.png
.\bin\adb.exe -s NAP shell rm /system/bin/capture.png
pause