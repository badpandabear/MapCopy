@echo off
if %1 == --quiet shift
FC /B %1 %2  > fc.out.txt
