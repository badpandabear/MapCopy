@echo off

set st=1

copy perm\test4%11.sav . > nul

..\mapcopy test4%11.sav -verbose +cv:CURRENT

cmp test4%11.sav perm\test4%11.001

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub2
goto done
:sub2

set st=2

copy perm\biggrass.mp . > nul

..\mapcopy biggrass.mp -verbose +fertility:calc

cmp biggrass.mp perm\test4.002

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub3
goto done
:sub3

set st=3

copy perm\mount.mp . > nul

..\mapcopy mount.mp -verbose +fertility:calcall

cmp mount.mp perm\test4.003

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub4
goto done

:sub4
set st=4

copy perm\biggrass.mp . > nul

..\mapcopy biggrass.mp -verbose +fertility:zero

cmp biggrass.mp perm\test4.004

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