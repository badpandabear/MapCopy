@echo off

set st=1

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup +seed -terrain -rs -improvement -visibility -ownership -bc -cr -F -cv 

fc /B test3%12.sav perm\test3%12.001 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub2
goto fail
:sub2

set st=2

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed +terrain -improvement -visibility -ownership -bc -cr -f -cv

fc /B test3%12.sav perm\test3%12.002 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub3
goto fail
:sub3

set st=3

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -rs +improvement -visibility -ownership -bc -cr -f -cv

fc /B test3%12.sav perm\test3%12.003 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub4
goto fail

:sub4
set st=4

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -rs -improvement +visibility -ownership -bc -cr -f -cv

fc /B test3%12.sav perm\test3%12.004 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub5
goto fail

:sub5
set st=5

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -rs -improvement -visibility +ownership -bc -cr -f -cv

fc /B test3%12.sav perm\test3%12.005 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub6
goto fail

:sub6
set st=6

copy perm\test3%1?.sav . > nul
copy perm\test3?.mp . > nul

..\mapcopy test31.mp test32.mp -verbose -backup -seed -terrain -rs -improvement -visibility -ownership -bc -cr -f -cv +cs

fc /B test32.mp perm\test3.006 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub7
goto fail

:sub7
set st=7

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -rs -improvement -visibility -ownership -bc -cr -f +cv

fc /B test3%12.sav perm\test3%12.007 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub8
goto fail

:sub8
set st=8

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -rs -improvement -visibility -ownership +bc -cr -f -cv

fc /B test3%12.sav perm\test3%12.008 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub9
goto fail

:sub9
set st=9

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -rs -improvement -visibility -ownership -bc +cr -f -cv

fc /B test3%12.sav perm\test3%12.009 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub10
goto fail

:sub10
set st=10

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -rs -improvement -visibility -ownership -bc -cr -f +cv:CURRENT

fc /B test3%12.sav perm\test3%12.010 > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed
goto fail


:fail
echo test 3.%st% failed
goto done

:passed
echo test3 passed

:done