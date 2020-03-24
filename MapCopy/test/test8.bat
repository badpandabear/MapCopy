@echo off

set st=1

copy perm\biggrass.mp . > nul
copy perm\test8mg1.sav . > nul

..\mapcopy biggrass.mp test8mg1.sav -verbose 

cmp test8mg1.sav perm\test8mg1.001

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail


set st=2

copy perm\test8mg1.sav . > nul

..\mapcopy test8mg1.sav temp.mp -verbose 

cmp temp.mp perm\test8.002

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=3

copy perm\test8fw1.sav . > nul
copy perm\test8mg1.sav . > nul

..\mapcopy test8mg1.sav test8fw1.sav -verbose 

cmp test8fw1.sav perm\test8fw1.003

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=4

copy perm\test8fw1.sav . > nul
copy perm\test8mg1.sav . > nul

..\mapcopy test8fw1.sav test8mg1.sav -verbose 

cmp test8mg1.sav perm\test8mg1.004

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

set st=5

:fail
echo test 8.%st% failed
goto done

:passed
echo test8 passed

:done
del temp.mp > nul