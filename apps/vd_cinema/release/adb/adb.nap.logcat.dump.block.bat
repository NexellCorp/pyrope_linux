@setlocal
@set YEAR=%date:~0,4%
@set MONTH=%date:~5,2%
@set DAY=%date:~8,2%
@set HOUR=%time:~0,2%
@set MINUTE=%time:~3,2%
@set SECOND=%time:~6,2%
@set POSTFIX=%YEAR%%MONTH%%DAY%_%HOUR%%MINUTE%%SECOND%

@echo If you want to stop log dump, plases press "Ctrl+C"
.\bin\adb.exe -s NAP logcat > .\result\logcat_%POSTFIX%.txt
