@echo off

call test1.bat

echo Testing FW...
call testfw.bat

echo Testing MG...
call testmg.bat

echo Testing cross compatibility...
call test8.bat

echo Testing city radius calculations...
call test9.bat

echo Testing fertility...
call test10.bat

echo Testing ToT Multimap copies...
call test11.bat