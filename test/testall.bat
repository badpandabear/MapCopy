@echo off

call test1.bat

echo Testing FW...
call testfw.bat

echo Testing MG...
call testmg.bat

echo Testing cross compatibility...
call test8.bat
