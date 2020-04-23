@ECHO OFF

rem Build the p8 platform library for Windows

SETLOCAL

SET MYDIR=%~dp0
SET BUILDARCH=%1
SET BUILDTYPE=%2
SET VSVERSION=%3
SET INSTALLPATH=%4
IF [%4] == [] GOTO missingparams

SET INSTALLPATH=%INSTALLPATH:"=%
SET BUILDTARGET=%INSTALLPATH%\cmake\%BUILDARCH%
SET TARGET=%INSTALLPATH%\%BUILDARCH%

CALL "%MYDIR%..\support\windows\cmake\generate.cmd" %BUILDARCH% nmake "%MYDIR%..\" "%BUILDTARGET%" "%TARGET%" %BUILDTYPE% %VSVERSION% static
CALL "%MYDIR%..\support\windows\cmake\build.cmd" %BUILDARCH% "%BUILDTARGET%" %VSVERSION%
GOTO exit

:missingparams
ECHO.%~dp0 requires 4 parameters
ECHO.  %~dp0 [architecture] [type] [version] [install path]
ECHO.
ECHO. architecture:    amd64 x86
ECHO. type:            Release Debug
ECHO. version:         Visual Studio version (2019)
ECHO. install path:    installation path without quotes
exit /b 99

:exit