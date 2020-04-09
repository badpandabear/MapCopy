@echo off

set st=1

copy perm\test4%11.sav . > nul

..\mapcopy test4%11.sav -verbose +cv:CURRENT

fc /B test4%11.sav perm\test4%11.001 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub2
goto done
:sub2

set st=2

copy perm\mount.mp . > nul

..\mapcopy mount.mp -verbose +rs:set

fc /B mount.mp perm\test4.002 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub3
goto done
:sub3

set st=3

..\mapcopy mount.mp -verbose +rs:clear

fc /B mount.mp perm\mount.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub4
goto done

:sub4
set st=4

copy perm\biggrass.mp . > nul

..\mapcopy biggrass.mp -verbose +fertility:zero

fc /B biggrass.mp perm\test4.004 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

set st=5

:fail
echo test 4.%st% failed
goto done

:passed
echo test4 passed

:done