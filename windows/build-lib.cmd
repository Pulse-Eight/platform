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

rem Check support submodule
IF NOT EXIST "%MYDIR%..\support\windows\cmake\build.cmd" (
  rem Try to init the git submodules
  cd "%MYDIR%.."
  git submodule update --init -r >nul 2>&1

  IF NOT EXIST "%MYDIR%..\support\windows\cmake\build.cmd" (
    ECHO.*** support git submodule has not been checked out ***
    ECHO.
    ECHO.See docs\README.windows.md
    EXIT /b 2
  )
)

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
