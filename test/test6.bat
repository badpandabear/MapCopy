@echo off

set st=1

copy perm\biggrass.mp . > nul
copy perm\test6%11.sav . > nul

..\mapcopy test6%11.sav biggrass.mp +cs -verbose > nul

if not errorlevel 1 goto fail

..\mapcopy biggrass.mp test6%11.sav +cs -verbose > nul

if not errorlevel 1 goto fail

:sub2

..\mapcopy test6%11.sav biggrass.mp +cv -verbose > nul

if not errorlevel 1 goto fail

..\mapcopy biggrass.mp test6%11.sav +cv -verbose > nul

if not errorlevel 1 goto fail


:sub3

set st=3

copy perm\mount.mp . > nul

..\mapcopy mount.mp test6%11.sav -verbose > nul

if not errorlevel 1 goto fail

..\mapcopy biggrass.mp mount.mp -verbose > nul

if not errorlevel 1 goto fail

..\mapcopy test6%11.sav mount.mp -verbose > nul

if not errorlevel 1 goto fail

:sub4
set st=4

..\mapcopy test6%11.sav +t -verbose > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

..\mapcopy biggrass.mp +f:adjust -verbose > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=5

FC /B biggrass.mp perm\biggrass.mp > nul
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

FC /B mount.mp perm\mount.mp > nul
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

FC /B test6%11.sav perm\test6%11.sav > nul
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

set st=6

:fail
echo test 6.%st% failed
goto done

:passed
echo test6 passed

:done
