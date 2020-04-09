@echo off

set st=1

copy perm\test8mg1.sav . > nul

..\mapcopy test8mg1.sav temp.mp -verbose 

fc /B temp.mp perm\test8.002 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=2

copy perm\test8fw1.sav . > nul
copy perm\test8mg1.sav . > nul

..\mapcopy test8mg1.sav test8fw1.sav -verbose 

fc /B test8fw1.sav perm\test8fw1.003 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=3

copy perm\test8fw1.sav . > nul
copy perm\test8mg1.sav . > nul

..\mapcopy test8fw1.sav test8mg1.sav -verbose 

fc /B test8mg1.sav perm\test8mg1.004 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=4
copy perm\test8tot1.sav . > nul
copy perm\test8fw.sav . > nul

..\mapcopy test8fw1.sav test8tot1.sav -verbose -backup 

fc /B test8tot1.sav perm\test8tot2.sav > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=5
copy perm\test8tot1.sav . > nul
copy perm\test8mg1.sav . > nul

..\mapcopy test8tot1.sav test8mg1.sav -verbose -backup

fc /B test8mg1.sav perm\test8mg1.005 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed


:fail
echo test 8.%st% failed
goto done

:passed
echo test8 passed

:done
del temp.mp > nul