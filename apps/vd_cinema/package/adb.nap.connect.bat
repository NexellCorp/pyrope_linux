@setlocal

@.\bin\adb.exe start-server > nul

@for /f "delims=" %%i in (nap.network.txt) do @(
	@if not %%i=="" (
		@.\bin\adb.exe connect %%i:5555
		echo adb network connected..
	)
)
