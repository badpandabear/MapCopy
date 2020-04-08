@echo off

set st=1

copy perm\test7%11.sav . > nul

..\test7 test7%11.sav > nul

if errorlevel 1 goto fail
if not errorlevel 0 goto fail

cmp test7%11.sav perm\test7%11.001

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

set st=2

:fail
echo test 7.%st% failed
goto done

:passed
echo test7 passed

:done
