README.TXT for MapCopy 1.beta1  
Copyright (C) 2000, James Dustin Reichwein, All Rights Reserved.

Contents

Introduction
File List
Command Line Arguments
Examples
Description of Features
Future Ideas 
Beta Report Form
Working with the Source Code


Introduction

  MapCopy is a utility for copying map information between Civ2 saved game files
  and saved map files.  It supports Fantastic Worlds and Multiplayer Gold 
  Edition files, and has been tested on on Windows 98 and Windows NT 4.0.  It 
  WILL NOT support MS-DOS or Windows 3.1. Nor will it support versions
  of Civ 2 prior to Fantastic Worlds, or TOT. It is able to copy resource seed, 
  terrain, improvement, visibility, ownership, civ specific view, fertility, and 
  other types of information between .SAV files and .MP files. It can also 
  calculate values for fertility if needed.  These features are described in 
  more detail below.

  This software is open source Freeware.  The source code is released under
  the Mozilla Public License (MPL), and is included in this archive. This 
  archive may also be distributed electronically.  If you choose to place this 
  archive on a web or FTP server, I would appreciate being notified so that I 
  can notify you of new versions.

  This software is BETA quality. I have tested it as much as I can, but there 
  are probably still problems with this software.  I therefore ask that you fill 
  out the Beta Report Form included in this document, EVEN IF YOU ENCOUNTER NO 
  PROBLEMS.  Reports from people successfully using this utility give me an 
  indication of how mature it has become, and also tell me if it is worth 
  continuing development on MapCopy.

  The most recent version of MapCopy can be found at: 
  http://home.san.rr.com/dustyr/download/mapcopy.zip

  For additional information on the contents of saved game file, I recommend
  Allard Höfelt's document at http://allard.8m.com/hexediting.htm. Without that
  document this utility would never have happened.

  The author of MapCopy can be contacted at jreichwe@san.rr.com

File List

  This archive contains the following files:
  mapcopy.exe    The executable.
  README.TXT     This text file.
  LICENSE.TXT    The MPL.
  src\           Source files
      mapcopy.cpp
      civ2sav.h
      civ2sav.cpp
      DustyUtil.h
      DustyUtil.cpp

Command Line Syntax. 

  Items in [square brackets] are optional. | means "or".

  mapcopy [/? | -h]  - Displays help information.

  mapcopy source dest [options]  - Copies information from source to dest.

  If the destination does not exist, and it is a .MP file, it will be created.

  mapcopy dest [options] - Provides in place modifications on dest.
 
  Options: +x turns option x on, -x turns option x off.
           -x:AAA or +x:AAA performs action AAA for an option.
	   All options are case insensitive.

    s[eed]        Copies the resource seed.
    t[errain]     Copies the terrain data.
    i[mprovement] Copies terrain improvements.
    v[isibility]  Copies terrain visibility.
    o[wnership]   Copies terrain owner ship.
    cs            Copies civilization start locations from an MP file.
    bc            Copies "body counter" values for continents.
    cr            Copies "city radius" data for terrain.
    v[erbose]     Enables informative screen messages. (on by default)
    b[ackup]      Creates of a backup named "dest.bak". (on by default)
    /? or -h      Displays help screen.
    f[ertility][:CALC|CALCALL|ADJUST|ZERO]
                  Copies or calculates fertility data for terrain.
		  :CALC     Calculates fertility data for grasslands and plains.
		  :CALCALL  Calculates fertility data for all terrain.
		  :ADJUST   Copies fertility data, adjusting for the presence of
		            cities.
                  :ZERO     Sets fertility data to 0 for all squares.
    cv[:CURRENT]  Copies civ specific visible terrain improvement data.
                  :CURRENT  Sets the civ specific visible improvement data so
                            that all civilizations see the most up to date
			    improvements.

    The default value of options is determined by the type of copy being 
    performed.  The below table describes their default values.

    Option      MP->MP   SAV->SAV   MP->SAV   SAV->MP,  MP,    SAV
    seed        COPY     COPY       COPY      COPY      OFF    OFF 
    terrain     COPY     COPY       COPY      COPY      OFF    OFF 
    improvement COPY     OFF        OFF       OFF       OFF    OFF 
    visibility  COPY     OFF        OFF       OFF       OFF    OFF 
    ownership   COPY     OFF        OFF       OFF       OFF    OFF 
    cs          COPY     OFF        OFF       OFF       OFF    OFF 
    bc          COPY     COPY       COPY      COPY      OFF    OFF 
    cr          COPY     OFF        OFF       OFF       OFF    OFF 
    verbose     ON       ON         ON        ON        ON     ON  
    backup      ON       ON         ON        ON        ON     ON  
    fertility   COPY     ADJUST     CALC      OFF       OFF    OFF
    cv          OFF      OFF        OFF       OFF       OFF    OFF 


Examples:

  mapcopy ca_b4000.sav ca_b4000.mp

    Extracts the seed, terrain, and body counter information from ca_b4000.sav
    to ca_b4000.mp.  If ca_b4000.mp exists, a backup file named ca_b4000.mp.bak
    will be created. If it does not exist, it will be created.

 mapcopy biggrass.mp ca_b4000.sav -s +t -backup -verb

    Copies the terrain and body counter information from biggrass.mp to
    ca_b4000.sav.  No backup will be created and extra information will not be
    printed to the screen.

  mapcopy ca_b4000.sav -f:CALCALL

    Calculates new fertility information for all terrain squares in ca_b4000.sav
    A backup file will be created.


Description of Features

Here is a brief description of the data MapCopy works with, and what 
calculations MapCopy can perform.  For more detailed information, see
Allard's hex editing document.

Resource Seed (+s)

The resource seed controls the placements of resources, huts, and grassland 
shields.  Note that this can also be set in the Map Editor.

Terrain (+t)

Terrain information includes terrain type (Dessert, Ocean, etc) and whether
there is a river on a square.  It also contains information to suppress 
resources in a square, if a resource would normally appear there.

Terrain Improvements (+i)

This includes irrigation, roads, railroads, mining, farmlands, fortresses,
and air bases.  It also contains pollution information, and information on
whether a unit or city is present.  Note that if unit and city information
is not present for a square, any units or cities on that square will not
be visible until the units move.

Visibility (+v)

This determines what civilizations have explored what squares.

Ownership (+o)

This determines what civilization owns a given square. A civilization owns
a square if the square is by a unit, or a city of that civilization.  Note that
a civilization may attack a unit on a square owned by another civilization,
even if that unit belongs to the attacking civilization.

City Start Locations (+cs)

This is the information stored in .MP files that determines where civilizations
start. This is not contained within .SAV files, so attempt to copy this 
information to or from a .SAV file will fail, and MapCopy will display an
error message.

Body Counter (+bc)

The Body Counter is an integer assigned to each continent.  It may be used
to determine if a trade route is between two continents or not.  You can see
this information by enabling cheat mode and positioning the cursor on a square.
The number next to the square's location is its body counter.

Note that if you are copying from a .MP file to a .SAV file, I recommend 
validating the .MP file before saving it.  This will cause the Map Editor 
to calculate Body Counters for continents.  Failure to do so may cause trade
routes to behave strangely.

City Radius (+cr)

This information determines which squares are within the city radius of a city
of a given civilization.   I'm not sure what this is used for.

Fertility (+f)

This information determines how desirable a square is for building a city, this
is used by the AI to pick city locations. Normally, only grasslands and plains
are considered fertile, so the AI won't build cities on other terrain types. The
fertility is usually considered low if a grassland or plains square is next
to a city.

When copying from a .MP file to a .SAV file, there is no fertility information
in the .MP file to copy.  To handle this, MapCopy can calculate a value for
fertility, similar to the values the game calculates.  By default, when
copying from a .MP file to a .SAV file, the +f:CALC option is on, which
calculates fertility information.  When copying from a .SAV to another 
.SAV file, the +f:ADJUST option is on. This copies fertility information,
but adjusts it to be lower when a city is nearby.

Additionally, the +f:CALCALL option will cause MapCopy to calculate a fertility
value for all terrain types. This should cause the AI to build cities on
non grassland/plain squares.  I don't know how this affects the AI's game 
playing ability.

The algorithm MapCopy uses for calculating fertility is shown below. This is
NOT the same algorithm used by Civ 2, and it is by no means optimal. Again,
I don't know how this affects the AI's game playing ability.

begin func calc_fertility(x, y)
{
    let resourceSum = 0
 
    for each square in the city radius of x, y
       // Note that the food/trade/shield potential for a square is the
       // maximum output for a square, including roads, rail roads, mining,
       // irrigation and farmlands, but excluding resources, and increases
       // due to city improvements or wonders (except supermarkets).
       resourceSum += 3 * the Food Potential for this terrain type
       resourceSum += 2 * the Trade Potential for this terrain type
       resourceSum += 1 * the Shield Potential for this terrain type
    end

    // The value 252 comes from 42 * 3 + 42 * 2 + 42. 42 was chosen as
    // a good value for a resource, representing 2 production from each square.
    // The 3, 2, and 1 are the weights used for food, trade, and shields
    // respectively. These weights were chosen from my experience with how
    // the AI does things.  Therefore, a city with 2 max food per square, 2 max 
    // trade per square, and 2 max shields per square would get a maximum score. 
    // Note, however, that other combinations can also yield a maximum score, 
    // with food being valued more highly than trade, and trade being valued
    // over shields.
    fertility = resourceSum * 15 / 252

    // The fertility information stored in the saved game files ranges
    // from 0 to 15.
    if fertility > 15 then fertility = 15

    // These conditions are to maintain some compatibility with the way
    // Civ2 calculates fertility

    if x, y is near a city then fertility = fertility - 8, or 0 if fertility < 8
    else if x, y is grassland or plains fertility = 8;

    return fertility
}

Civ Specific View (+cv)

The Civ Specific View information includes what terrain improvements a 
civilization has most recently seen at any given square.  Because of this 
information, you must constantly re-explore territory to see what improvements
other civilizations have been up to. The +cv:CURRENT option causes 
MapCopy to allow every civilization to see the most up to date information.

Note that Civ Specific View information is not stored in .MP files, attempts
to copy this information to or from a .MP file will fail, causing MapCopy to
print an error message.

Future Ideas 

Below are ideas for future improvements to MapCopy, or for future Civ 2
related utilities. Let me know if you like these ideas.

1.  Add the ability to copy units and cities to MapCopy.

2.  I've considered a command line utility for scenario writers that would
    accept a script file for altering saved games. For example, the following
    script might cut every Roman citiy's population in half.

        foreach city do
	    if city.owner = Romans then
	       city.population = city.population / 2
	    endif
	endfor

3.  Of course, I could always write The One True Civ2 Scenario Editor, which
    would provide a nice GUI for editing all features of maps, cities, units, 
    events, etc.  Of course, by the time it gets finished Civ 5 will be out.

Beta Report Form

  This is BETA quality software.  To iron out any wrinkles that are left in it,
  I need feedback from users.  If you have used this beta version of MapCopy,
  I would appreciate it if you would fill out the below form and e-mail it
  to jreichwe@san.rr.com.  Please fill it out even if you encounter no problems
  with MapCopy.  This allows me to tell if MapCopy is mature enough for a final
  release, or if MapCopy is not used by anyone (and therefore should not be
  developed further).

  ------------------------------------------------------------------------------
  MapCopy v1.beta1 Beta Report Form

  Do you find MapCopy to be useful?

  Are there any features you wish it had?

  Are there any features you wish it didn't have?

  Please Describe any problems you've had with MapCopy. Include bugs in MapCopy,
  mistakes in the documentation, or strange Game/AI behavior on any games modified 
  by MapCopy.  For each problem please include:

   * A detailed description of the problem
   * Indicate which version of Civ 2 you are working with (MGE, or FW). Note
     that MapCopy will not work with versions of Civ 2 prior to FW, nor will
     it work with TOT.
   * A description of how to reproduce the problem.  If possible include any
     .MP or .SAV files that are causing problems.
   
  Any Other Comments?

  ------------------------------------------------------------------------------

Working with the Source Code

  MapCopy is open source software. This means that the source code for MapCopy
  is included in this distribution. You can compile MapCopy for yourself, or 
  use, modify, or distribute its source code as stated by the Mozilla Public
  License (see LICENSE.TXT).

  You may use the Civ2Sav.cpp or DustyUtil.cpp files for you own projects, as
  stated in the MPL. If you do so, keep in mine that this code is still in its
  early phases, and that I may drastically change it as needed.

  On the other hand, if you make an improvement to any of the MapCopy source 
  code, I would appreciate it if you would send the new code to me, so I could
  incorporate it into future versions.

  Building MapCopy

    MapCopy is written to be fully ANSI C++ compliant. Unfortunately, it is hard
    to find fully ANSI C++ compliant compilers. The compiler I used is 
    the free Borland C++ 5.5 compiler, available at 
    http://www.borland.com/bcppbuilder/freecompiler/.  This compiler has a bug
    in its fstream library, in the fstream.cc file.  I was able to fix this bug,
    but the code is copyrighted so that I cannot redistribute it.  As a result,
    I have not enclosed a Makefile.  You may find that another C++ compiler may
    compile MapCopy without problems. Be alert for compatibilities with the
    STL or string class.

    If you wish to compile MapCopy using the Borland C++ 5.5 compiler, e-mail
    me at jreichwe@san.rr.com and I will send you the changes I made to
    fstream.cc.

Good Luck, and I hope you find this software useful!
