@echo off

set VERSION_FILE=version.h

for /f "tokens=3" %%a in ('findstr MAJOR %VERSION_FILE%') do set MAJOR=%%a
for /f "tokens=3" %%a in ('findstr MINOR %VERSION_FILE%') do set MINOR=%%a
for /f "tokens=3" %%a in ('findstr BUILD %VERSION_FILE%') do set BUILD=%%a

set /a BUILD=%BUILD%+1

(
echo #ifndef PLLDRV_VERSION_H
echo #define PLLDRV_VERSION_H
echo.
echo #define PLLDRV_VERSION_MAJOR %MAJOR%
echo #define PLLDRV_VERSION_MINOR %MINOR%
echo #define PLLDRV_VERSION_BUILD %BUILD%
echo.
echo #define PLLDRV_VERSION_STRING "%MAJOR%.%MINOR%.%BUILD%"
echo.
echo #endif
) > %VERSION_FILE%

echo Version bumped to %MAJOR%.%MINOR%.%BUILD%