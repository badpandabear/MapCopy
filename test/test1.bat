@echo off
..\mapcopy > test.txt
cmp --quiet test.txt perm\test1.001
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

:fail
echo test 1 failed
goto done

:passed
echo test1 passed


:done
