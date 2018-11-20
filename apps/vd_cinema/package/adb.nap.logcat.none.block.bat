@setlocal

@.\bin\adb.exe start-server > nul

@for /f %%i in ('.\bin\adb.exe devices^|findstr /e "device"') do @(
	@for /f %%j in ('echo %%i^|findstr /x "NAP"') do @(
		@set NX_DEVICE_USB=%%j
	)
	@for /f %%j in ('echo %%i^|findstr /r "[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\:[0-9][0-9][0-9]*"') do @(
		@set NX_DEVICE_NETWORK=%%j
	)
)

@if not "%NX_DEVICE_NETWORK%"=="" (
	@set NX_DEVICE=%NX_DEVICE_NETWORK%
) else (
	@if not "%NX_DEVICE_USB%"=="" (
		@set NX_DEVICE=%NX_DEVICE_USB%
	)
)

@if "%NX_DEVICE%"=="" (
	@echo adb device not connected.
	@goto:eof
)

@echo ^>^> ADB Target Device : %NX_DEVICE%

@for /f "delims=" %%i in ('.\bin\adb.exe -s %NX_DEVICE% root') do @(
	@for /f %%j in ('echo %%i^|findstr /v /c:"adbd is already running as root"') do @(
		@echo change root permission..
		@timeout /t 5 /nobreak > nul
	)
)

@.\bin\adb.exe -s %NX_DEVICE% logcat -d
@pause
