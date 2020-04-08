
BCC = H:\BCC55
INCLUDE = $(BCC)\INCLUDE
LIB = $(BCC)\LIB

CC = bcc32 -a1 -d -O2 -WC -I$(INCLUDE);..\DustyUtil -L$(LIB)
CCDEBUG = bcc32 -a1 -Od   -WC -y -I$(INCLUDE);..\DustyUtil -L$(LIB)

mapcopy:  mapcopy.obj Civ2Sav.obj DustyUtil.obj
	$(CC) mapcopy.obj Civ2Sav.obj DustyUtil.obj

mapcopy.obj: mapcopy.cpp Civ2Sav.h ..\DustyUtil\DustyUtil.h
	$(CC)  -c mapcopy.cpp

Civ2Sav.obj: Civ2Sav.cpp Civ2Sav.h ..\DustyUtil\DustyUtil.h
	$(CC)  -c Civ2Sav.cpp

DustyUtil.obj: ..\DustyUtil\DustyUtil.h ..\DustyUtil\DustyUtil.cpp
	$(CC)  -c ..\DustyUtil\DustyUtil.cpp


debug: mapcopyd.obj Civ2Savd.obj DustyUtild.obj
	$(CCDEBUG) mapcopy.obj Civ2Sav.obj DustyUtil.obj

mapcopyd.obj: mapcopy.cpp Civ2Sav.h ..\DustyUtil\DustyUtil.h
	$(CCDEBUG) -c mapcopy.cpp

Civ2Savd.obj: Civ2Sav.cpp Civ2Sav.h ..\DustyUtil\DustyUtil.h
	$(CCDEBUG) -c Civ2Sav.cpp

DustyUtild.obj: ..\DustyUtil\DustyUtil.h ..\DustyUtil\DustyUtil.cpp
	$(CCDEBUG) -c ..\DustyUtil\DustyUtil.cpp

clean:
	del *.exe *.obj *.ilc *.ild *.ilf *.ils *.mbt *.mrt *.OBR *.tds     

test7: test7.obj Civ2Sav.obj DustyUtil.obj
	$(CC) test7.obj Civ2Sav.obj DustyUtil.obj

test7.obj: test7.cpp Civ2Sav.h
	$(CC) -c test7.cpp

