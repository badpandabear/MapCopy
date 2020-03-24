@echo off

set st=1

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup +seed -terrain -improvement -visibility -ownership -bc -cr -F -cv 

cmp test3%12.sav perm\test3%12.001

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub2
goto done
:sub2

set st=2

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed +terrain -improvement -visibility -ownership -bc -cr -f -cv

cmp test3%12.sav perm\test3%12.002

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub3
goto done
:sub3

set st=3

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain +improvement -visibility -ownership -bc -cr -f -cv

cmp test3%12.sav perm\test3%12.003

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub4
goto done

:sub4
set st=4

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -improvement +visibility -ownership -bc -cr -f -cv

cmp test3%12.sav perm\test3%12.004

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub5
goto done

:sub5
set st=5

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -improvement -visibility +ownership -bc -cr -f -cv

cmp test3%12.sav perm\test3%12.005

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub6
goto done

:sub6
set st=6

copy perm\test3%1?.sav . > nul
copy perm\test3?.mp . > nul

..\mapcopy test31.mp test32.mp -verbose -backup -seed -terrain -improvement -visibility -ownership -bc -cr -f -cv +cs

cmp test32.mp perm\test3.006

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub7
goto done

:sub7
set st=7

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -improvement -visibility -ownership -bc -cr -f +cv

cmp test3%12.sav perm\test3%12.007

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub8
goto done

:sub8
set st=8

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -improvement -visibility -ownership +bc -cr -f -cv

cmp test3%12.sav perm\test3%12.008

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub9
goto done

:sub9
set st=9

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -improvement -visibility -ownership -bc +cr -f -cv

cmp test3%12.sav perm\test3%12.009

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub10
goto done

:sub10
set st=10

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -improvement -visibility -ownership -bc -cr -f +cv:CURRENT

cmp test3%12.sav perm\test3%12.010

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub11
goto done

:sub11
set st=11

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -improvement -visibility -ownership -bc -cr -f:CALC -cv

cmp test3%12.sav perm\test3%12.011

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub12
goto done

:sub12
set st=12

copy perm\test3%1?.sav . > nul

..\mapcopy test3%11.sav test3%12.sav -verbose -backup -seed -terrain -improvement -visibility -ownership -bc -cr -f:CALCALL -cv

cmp test3%12.sav perm\test3%12.012

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub13
goto done

:sub13
set st=13

copy perm\test3%1?.sav . > nul

..\mapcopy biggrass.mp test3%12.sav -verbose -backup -seed -terrain -improvement -visibility -ownership -bc -cr -fertility:ADJUST -cv

cmp test3%12.sav perm\test3%12.013

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub14
goto done

:sub14
set st=14

copy perm\test3%1?.sav . > nul

..\mapcopy biggrass.mp test3%12.sav -verbose -backup -seed -terrain -improvement -visibility -ownership -bc -cr +f -cv

cmp test3%12.sav perm\test3%12.014

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto sub15
goto done

:sub15
set st=15

copy perm\test3%1?.sav . > nul

..\mapcopy biggrass.mp test3%12.sav -verbose -backup -seed -terrain -improvement -visibility -ownership -bc -cr +fertility:zero -cv

cmp test3%12.sav perm\test3%12.015

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

set st=16

:fail
echo test 3.%st% failed
goto done

:passed
echo test3 passed

:done