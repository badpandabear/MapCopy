@echo off

copy perm\grass_flat.mp grass_f.mp > nul
copy perm\grass_round.mp grass_r.mp > nul

call test9_1.bat f 0
call test9_1.bat f 1
call test9_1.bat f 2
call test9_1.bat f 3
call test9_1.bat f 4


call test9_1.bat r 0
call test9_1.bat r 1
call test9_1.bat r 2
call test9_1.bat r 3
call test9_1.bat r 4

del grass_f.mp > nul
del grass_r.mp > nul

del gs_f_*.mp > nul
del gs_r_*.mp > nul

