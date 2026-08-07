@echo off

set OUT=C:\temp\out

if not exist %OUT% mkdir %OUT%
copy /Y objfre_wxp_x86\i386\plldrv.sys %OUT%\
copy /Y sources %OUT%\

echo Artifacts copied to %OUT%