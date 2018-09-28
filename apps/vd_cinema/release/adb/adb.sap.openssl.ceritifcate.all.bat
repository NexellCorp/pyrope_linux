.\bin\adb.exe -s SAP shell openssl x509 -text -in /mnt/mmc/bin/cert/ca.self-signed.pem
.\bin\adb.exe -s SAP shell openssl x509 -text -in /mnt/mmc/bin/cert/intermediate.signed.pem
.\bin\adb.exe -s SAP shell openssl x509 -text -in /mnt/mmc/bin/cert/leaf.signed.pem
pause