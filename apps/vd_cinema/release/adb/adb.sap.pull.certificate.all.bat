.\bin\adb.exe -s SAP pull /mnt/mmc/bin/cert/ca.self-signed.pem .\result\ca.self-signed.pem
.\bin\adb.exe -s SAP pull /mnt/mmc/bin/cert/intermediate.signed.pem .\result\intermediate.signed.pem
.\bin\adb.exe -s SAP pull /mnt/mmc/bin/cert/leaf.signed.pem .\result\leaf.signed.pem
pause