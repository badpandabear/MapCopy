@echo off

set st=1
..\mapcopy > test.txt
fc /B test.txt perm\test1.001 > nul
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=2
copy perm\mount.mp . > nul
del test1.mp > nul
..\mapcopy mount.mp test1.mp > test.txt

fc /B test.txt perm\test1.002 > nul
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=3
copy perm\tot_multiple1.sav . > nul
copy perm\tot_map1.mp . > nul

..\mapcopy tot_map1.mp tot_multiple1.sav +verb > test.txt

fc /B test.txt perm\test1.003 > nul
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=4
..\mapcopy mount.mp test1.mp -verbose > test.txt
fc /B test.txt perm\test1.004 > nul
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=5

..\mapcopy tot_map1.mp tot_multiple1.sav +verb:DEV > test.txt

fc /B test.txt perm\test1.005 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

:fail
echo test 1.%st% failed
goto done

:passed
echo test1 passed


:done
del test.txt mount.mp test1.mp tot*.* > nul