@echo off

set st=1

copy perm\test2%1?.sav . > nul
del test2%12.sav.bak > nul

..\mapcopy test2%11.sav test2%12.sav +s +t +i +v +o +bc +cr +f +cv -verbose

cmp test2%12.sav test2%11.sav

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub2
goto :fail

:sub2
set st=2

cmp test2%12.sav.bak perm\test2%12.sav

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub3
goto :fail

:sub3
set st=3

copy perm\test2fw?.mp . > nul

..\mapcopy test2fw1.mp test2fw2.mp +s +t +i +v +o +bc +cr +f +cs -verbose -backup

cmp test2fw1.mp test2fw2.mp

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

set st=4

:fail
echo test 2.%st% failed
goto done

:passed
echo test2 passed


:done
