"c:\Program Files (x86)\Inno Setup 6\ISCC.exe" "%~dp0"%distrib.iss"

@echo off
if not %ERRORLEVEL%==0 (
	if "%DO_NOT_PAUSE%" == "0" (
		echo Failed!
		pause
	)
	endlocal
	exit /b 1
)

endlocal

pause
exit /b 0
