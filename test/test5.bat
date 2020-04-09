@echo off

set st=1

copy perm\biggrass.mp . > nul

..\mapcopy biggrass.mp temp.mp -verbose

fc /B biggrass.mp temp.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub2
goto done
:sub2

set st=2

copy perm\test5%11.sav . > nul
copy perm\biggrass.mp . > nul

..\mapcopy biggrass.mp test5%11.sav -verbose 

fc /B test5%11.sav perm\test5%11.002 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub3
goto done
:sub3

set st=3

del temp.mp > nul
copy perm\test5%11.sav . > nul

..\mapcopy test5%11.sav temp.mp -verbose

fc /B temp.mp perm\test5.003 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub4
goto done

:sub4
set st=4

copy perm\test5%1?.sav . > nul

..\mapcopy test5%11.sav test5%12.sav -verbose

fc /B test5%12.sav perm\test5%12.004 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

set st=5

:fail
echo test 5.%st% failed
goto done

:passed
echo test5 passed

:done
del temp.mp > nul