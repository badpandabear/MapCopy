@echo off

set st=1

copy perm\test_fert.mp tf.mp > nul


..\mapcopy tf.mp +f:CALC -verbose -b

fc /B tf.mp perm\tf1.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub2
goto :fail

:sub2
set st=2

copy perm\test_fert.mp tf2.mp > nul
copy perm\test_fert_city.sav tfc.sav > nul

..\mapcopy tf2.mp tfc.sav -verbose -b

fc /B  tfc.sav perm\tfc1.sav > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub3
goto :fail

:sub3
set st=3

copy perm\test_fert_city.sav tfc2.sav > nul

..\mapcopy tf.mp tfc2.sav +f:ADJUST -verbose -backup

fc /B tfc2.sav perm\tfc1.sav > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub4
goto :fail

:sub4
set st=4

..\mapcopy tf.mp tf3.mp +f:ZERO -verbose -backup

fc /B tf3.mp perm\test_fert.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub5
goto :fail


:sub5
set st=5

copy perm\test_calcall.mp tc.mp > nul

..\mapcopy tc.mp tc1.mp +f:CALCALL -verbose -backup

fc /B tc1.mp perm\tc1.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed
goto :fail

:fail
echo test 10.%st% failed
goto done

:passed
echo test10 passed


:done
del tf.mp tf2.mp tf3.mp tfc.sav tfc2.sav tc.mp tc1.mp
