@echo off
rem WDK environment is already active
rem Run this script from WDK Build Environment

build -cZ

if errorlevel 1 goto error

call postbuild.cmd
call increment_version_number.cmd

goto end


:error
echo BUILD FAILED
exit /b 1


:end
echo DONE