@echo off

set st=1

..\radius_test grass_%1.mp gs_%1_0_0_%2.mp 0 0 %2
fc /B gs_%1_0_0_%2.mp perm\gs_%1_0_0_%2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=2

..\radius_test grass_%1.mp gs_%1_0_118_%2.mp 0 118 %2
fc /B gs_%1_0_118_%2.mp perm\gs_%1_0_118_%2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=3

..\radius_test grass_%1.mp gs_%1_1_119_%2.mp 1 119 %2
fc /B gs_%1_1_119_%2.mp perm\gs_%1_1_119_%2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=4

..\radius_test grass_%1.mp gs_%1_149_119_%2.mp 149 119 %2
fc /B gs_%1_149_119_%2.mp perm\gs_%1_149_119_%2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=5

..\radius_test grass_%1.mp gs_%1_149_1_%2.mp 149 1 %2
fc /B gs_%1_149_1_%2.mp perm\gs_%1_149_1_%2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=6

..\radius_test grass_%1.mp gs_%1_148_0_%2.mp 148 0 %2
fc /B gs_%1_148_0_%2.mp perm\gs_%1_148_0_%2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=7

..\radius_test grass_%1.mp gs_%1_0_48_%2.mp 0 48 %2
fc /B gs_%1_0_48_%2.mp perm\gs_%1_0_48_%2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if not errorlevel 0 goto fail

set st=8

..\radius_test grass_%1.mp gs_%1_75_57_%2.mp 75 57 %2
fc /B gs_%1_75_57_%2.mp perm\gs_%1_75_57_%2.mp > nul

if errorlevel 2 goto fail
if errorlevel 1 goto fail
if errorlevel 0 goto passed

:fail
echo test 9_%1_%2_%st% failed
goto done

:passed
echo test9_%1_%2 passed

:done

