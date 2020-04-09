@echo off

set st=1

copy perm\tot_single.sav tot_single.sav > nul

..\mapcopy tot_single.sav tot_map1.mp -verbose -b

fc /B tot_map1.mp perm\tot_map1.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub2
goto :fail

:sub2
set st=2

..\mapcopy tot_map1.mp tot_single.sav -verbose -b

fc /B tot_single.sav perm\tot_single2.sav > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub3
goto :fail

:sub3
set st=3

copy perm\tot_single.sav tot_single.sav > nul

..\mapcopy tot_single.sav tot_map2.mp +i +v +o +cr +f -verbose -b

fc /B tot_map2.mp perm\tot_map2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub4
goto :fail

:sub4
set st=4

..\mapcopy tot_map2.mp tot_single.sav +i +v +o +cr +f -verbose -backup

fc /B tot_single.sav perm\tot_single.sav > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub5
goto :fail


:sub5
set st=5

copy perm\tot_single.sav tot_multiple1.sav > nul

..\mapcopy tot_map2.mp tot_multiple1.sav +i +v +o +cr +f +dm:2 -verbose -backup
..\mapcopy tot_map2.mp tot_multiple1.sav +i +v +o +cr +f +dm:3 -verbose -backup
..\mapcopy tot_map2.mp tot_multiple1.sav +i +v +o +cr +f +dm:4 -verbose -backup

fc /B tot_multiple1.sav perm\tot_multiple1.sav > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub61
goto :fail

:sub61
set st=6-1

..\mapcopy tot_multiple1.sav tot_multi1.mp +i +v +o +cr +f +sm:1 -verbose -backup
..\mapcopy tot_multiple1.sav tot_multi2.mp +i +v +o +cr +f +sm:2 -verbose -backup
..\mapcopy tot_multiple1.sav tot_multi3.mp +i +v +o +cr +f +sm:3 -verbose -backup
..\mapcopy tot_multiple1.sav tot_multi4.mp +i +v +o +cr +f +sm:4 -verbose -backup

fc /B tot_map2.mp tot_multi1.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub62
goto :fail

:sub62
set st=6-2

fc /B tot_map2.mp tot_multi2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub63
goto :fail

:sub63
set st=6-3

fc /B tot_map2.mp tot_multi3.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub64
goto :fail

:sub64
set st=6-4

fc /B tot_map2.mp tot_multi4.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub7
goto :fail

:sub7
set st=7

copy perm\tot_scifi1.mp . > nul

..\mapcopy tot_scifi1.mp tot_multiple1.sav +i +v +o +cr +f +dm:3 -verbose -backup

fc /B tot_multiple1.sav perm\tot_multiple2.sav > nul
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub8

goto :fail

:sub8
set st=8

copy perm\tot_scifi-1.sav . > nul
copy perm\tot_single.sav . > nul

..\mapcopy tot_scifi-1.sav tot_single.sav +i +v +o +cr +f -verbose -backup

fc /B tot_single.sav perm\tot_multiple3.sav > nul
if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub9

goto :fail

:sub9
set st=9

copy perm\tot_map1.mp . > nul
..\mapcopy tot_map1.mp tot_scifi-1.sav +i +v +o +cr +f +dm:ALL -verbose -backup

fc /B tot_scifi-1.sav perm\tot_scifi-2.sav > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub10

goto :fail

:sub10
set st=10

copy perm\tot_scifi-1.sav . > nul
copy perm\tot_multiple1.sav . > nul

..\mapcopy tot_scifi-1.sav tot_multiple1.sav +i +v +o +cr +f -verbose -backup

fc /B tot_multiple1.sav perm\tot_multiple4.sav > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

goto :fail


:fail
echo test 11.%st% failed
goto done

:passed
echo test11 passed


:done
del tot*.*