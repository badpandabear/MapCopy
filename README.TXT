README.TXT for MapCopy 1.2
Copyright (C) 2000,2005, James Dustin Reichwein, All Rights Reserved.

Contents

What's New
Introduction
Compatibility
I would like to thank...
File List
Command Line Arguments
Examples
Description of Features
Future Ideas 
Reporting Bugs
Working with the Source Code

What's New

  This version of MapCopy adds an improved formula for calculating fertility,
  support for saved games from versions of Civ2 prior to Fantastic Worlds, and 
  support for Test of Time saved games (including multiple map support). 
  It also can copy or modify the resource suppression flag separately from the
  terrain type. See below for more.


  1.2:   Improved fertility calculations, including using a 45 square radius
         when adjusting for the presence of a city. 

         Made the output less cluttered.  

         Included fertility tools.

         ToT support

Introduction

  MapCopy is a utility for copying map information between Civ2 saved game files
  and saved map files.  It is able to copy resource seed, terrain, improvement,
  visibility, ownership,  civ specific view, fertility, and other types of 
  information between .SAV files and .MP files. It can also calculate values for
  fertility if needed. These features are described in more detail below.

  This software is open source.  The source code is released under
  the Mozilla Public License (MPL), which is included in this archive in the
  LICENSE.TXT file. This archive may be distributed freely on physical 
  media or electronically.

  The author of MapCopy can be contacted at jreichwe@san.rr.com, or as "Dusty"
  at the CivFanatics forums (http://www.civfanatics.com/) and Apolyton forums 
  (http://apolyton.net/).

  The most recent version of MapCopy can be found at: 
  http://home.san.rr.com/dustyr/download/mapcopy.zip

  For additional information on the contents of saved game file, I recommend
  AllardHöfelt's hex editing document. The latest version (that I could find)
  of this is at Mercator's Civilization 2 Site 
  (http://www.civgaming.net/mercator/) in the Reference section.

Compatibility

  MapCopy is compatible with all versions of Civ2. It has been tested on 
  Windows XP, Windows XP SP2, Windows NT 4.0, Windows 98, and Windows 95.
  It WILL NOT support MS-DOS or Windows 3.1.  Please note that for some
  versions of Windows 95 MapCopy may complain that it cannot find the file
  "MSVCRT.DLL".  This file is required for MapCopy, and can be found in 
  later versions of Windows or can be downloaded from a variety of sites.
  I found a working copy at "http://DLL-files.com".

I would like to thank...

  Allard Höfelt for making the hex editing document.

  Jorrit "Mercator" Vermeiren for adding adding additional functionality to 
  MapCopy and for providing much information about ToT saved game files.

  Wobbegong (aka Catfish) from the CivFanatics forum for insight into how Civ2
  calculates fertility.

  And my imaginary friends. Except for Molly. Who is mean to me. WHAT? Why are
  you looking at me like that?
 

File List

  This archive contains the following files:
  MapCopy.exe    The executable.
  README.TXT     This text file.
  LICENSE.TXT    The MPL.
  src\           Source files
      mapcopy.cpp
      civ2sav.h
      civ2sav.cpp
      civ2map.cpp
      DustyUtil.h
      DustyUtil.cpp
      getfert.cpp
      FertDiff.cpp
  fertility\
      GetFert.exe
      FertDiff.exe

Command Line Syntax. 

  Items in [square brackets] are optional. | means "or".

  mapcopy [/? | -h]  - Displays help information.

  mapcopy source dest [options]  - Copies information from source to dest.

  If the destination does not exist, and it is a .MP file, it will be created.

  mapcopy dest [options] - Provides in place modifications on dest.
 
  Options: +x turns option x on, -x turns option x off.
           -x:AAA or +x:AAA performs action AAA for an option.
  All options are case insensitive. 

    s[eed]          Copies the resource seed.
    t[errain]       Copies the terrain data.
    i[mprovement]   Copies terrain improvements.
    v[isibility]    Copies terrain visibility.
    o[wnership]     Copies terrain owner ship.
    cs              Copies civilization start locations from an MP file.
    bc              Copies "body counter" values for continents.
    cr              Copies "city radius" data for terrain.
    ver[bose][:DEV] Enables informative screen messages. (on by default)
                    :DEV      Enables extra messages mainly meant for debugging
                              mapcopy.
    b[ackup]        Creates of a backup named "dest.bak". (on by default)
    /? or -h        Displays help screen.
    f[ertility][:CALC|CALCALL|ADJUST|ZERO]
                    Copies or calculates fertility data for terrain.
                    :CALC     Calculates fertility data for grasslands and 
                              plains.
                    :CALCALL  Calculates fertility data for all terrain.
                    :ADJUST   Copies fertility data, adjusting for the presence
                              of cities.
                    :ZERO     Sets fertility data to 0 for all squares.
    cv[:CURRENT]    Copies civ specific visible terrain improvement data.
                    :CURRENT  Sets the civ specific visible improvement data so
                              that all civilizations see the most up to date
                              improvements.
    rs[:SET|:CLEAR] Copies, sets, or clears resource suppression.
    sm:n | sm:ALL   Specifies which map in a multimap ToT saved game is being
                    copied from. "n" can range from 1 to 4. Only map 1 is 
                    allowed if the source is not a ToT saved game.  
                    "ALL" specifies all maps in a ToT saved game.
    dm:n | dm:ALL   Specifies which map in a multimap ToT saved game is being
                    copied to. "n" can range from 1 to 4. Only map 1 is allowed 
                    if the destination is not a ToT saved game. 
                    "ALL" will copy over all maps.  See Multimap Copies below
                    for more information.

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
    rs          COPY     COPY       COPY      COPY      OFF    OFF

    For copies between two ToT saved games the sm and dm options default to 
    "ALL".  In all other cases they default to 1.

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

  mapcopy ca_b4000.sav ca_a4000.sav -sm:2 -dm:4

     Copies the second map from a ToT saved game to the fourth map in another
     ToT saved game.
   
  mapcopy ca_b4000.sav +sm:1 +dm:2 -rs:CLEAR

      Copies the first map in a ToT saved game to the second map in the same
      saved game. (If there is no second map, this will create one). 
      This also clears the resource suppression flag from all squares in the
      second map, so all resources will be visible. 

Description of Features

Here is a brief description of the data MapCopy works with, and what 
calculations MapCopy can perform.  For more detailed information, see
Allard's hex editing document.

Resource Seed (+s)

The resource seed controls the placements of resources, huts, and grassland 
shields.  Note that this can also be set in the Map Editor.  In ToT files,
each map has its own resource seed, which is also copied by the +s option.

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
routes to behave strangely. (For additional information for ToT saved games
see the Multi-Map Copies section below).

City Radius (+cr)

This information determines which squares are within the city radius of a city
of a given civilization.   I'm not sure what this is used for.

Resource Suppression (+rs)

This information determines whether a resource that would normally be on the
square is made invisible.  For example, this bit prevents whales and fish
from being seen in the middle of the ocean.  +rs:SET will set this bit for
all squares, eliminating all special resources from the game. +rs:CLEAR will
clear this bit, allowing all special resources to be seen for all squares.

Civ Specific View (+cv)

The Civ Specific View information includes what terrain improvements a 
civilization has most recently seen at any given square.  Because of this 
information, you must constantly re-explore territory to see what improvements
other civilizations have been up to. The +cv:CURRENT option causes 
MapCopy to allow every civilization to see the most up to date information.

Note that Civ Specific View information is not stored in .MP files, attempts
to copy this information to or from a .MP file will fail, causing MapCopy to
print an error message.


Multi-Map Copies (+sm/+dm)

Civilization 2: Test of Time allows games to contain multiple maps. MapCopy 
supports copying into an arbitrary map of a ToT saved game (set with the +dm 
option) from a pre-ToT saved game, a .MP file, or an arbitrary map of another
ToT saved game (set with the +sm) options.  You can also copy all of the maps
from ToT saved game to another using "+sm:ALL +dm:ALL", which is the default
when copying from one ToT saved game to another. You can also copy over all 
maps in a ToT saved game from a single map in a ToT saved game, 
a pre-ToT saved game, or a .MP file using "+sm:n +dm:ALL".

In cases where the specified map number does not exist in the destination, 
it will be added to the destination if the number would not create a "gap" in
the destination file. For example, if "+sm:1 +dm:3" is used, and the
destination file has two maps, then a new map will be added in position 3.
If the destination file only had one map, it would be an error because there
would be a gap in map position 2.  Maps will also be created in the destination
file if "ALL" is used and the destination file has fewer maps than the source
file.  If the destination has more, then the extra maps in the destination will
not be affected.

In ToT, the body counter of continents depends on what map it is on. For map 1,
they range from 0 to 63. For map 2, 64-127, map 3, 128-191, and map 4, 192-255.
So in doing a multimap copy MapCopy will adjust the body counter according to 
what map position is being written to in the destination file.

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
playing ability.  Note that CALC and CALCALL also adjust their fertility
results based on the presence of a near by city.

I have attempted to come up with a formula for fertility that is as close to
how Civ 2 works as I can.  However, I did not have time to figure out how 
resources work.  The following algorithm is what I came up with, and seems to 
work well for maps with all special resources suppressed. I'm somewhat skeptical 
of some of the hard coded constants in this algorithm (especially the 2/3rds 
bonus for irrigation), and it may be that I'm one test case short of finding an 
exception to this algorithm. 

begin func calc_fertility(x, y)
{
    Set selfFood, selfShields, and selfTrade to be the food, shields, and trade 
    production for the square x,y. 
    (See notes below on how production for a square is calculated).

    Set innerFood, innerShields, and innerTrade to be the total food, shields, 
    and trade production of the 8 adjacent to x,y.

    Set outerFood, outerShields, and outerTrade to be the total food, shields,
    and trade production of the 12 squares in the outer ring of the city radius
    around x,y.

    Let combinedFood    = (selfFood    * 4) + (innerFood    * 2) + outerFood
    Let combinedShields = (selfShields * 4) + (innerShields * 2) + outerShields
    Let combinedTrade   = (selfTrade   * 4) + (innerTrade   * 2) + outerTrade

    Let fertility = (combinedFood * 3) + (combinedShields * 2) + (combinedTrade)
    Let fertility = fertility / 16

    Round fertility to the nearest integer.

    If the square at x,y is a grassland square without a shield then reduce
    fertility by one.

    If fertility < 8 then set fertility to 8.
    If fertility > 15 then set fertility to 15.

    return fertility
}

General Notes:

* All calculations are floating point up until the fertility is rounded to
the nearest integer.

* After calculating fertility, mapcopy performs the "ADJUST" operation, which
will subtract 8 from the fertility if it is >= 8 and nearby a city. "nearby"
means within the nearest 45 square radius of the city. (Think the city radius
with another ring added).

Notes on calculating production:

Civ 2 seems to start with the values of food, shield and trade production in
RULES.TXT and then modify them as follows:

If a square can be irrigated it gets a (2/3) boost to its food production. 
(That's the full floating point value 2/3, 1.67 does not seem to work).  Note
the value of the irrigation bonus does not matter, it can be zero and the 
production gets a boost for the purposes of fertility.

If a square can be mined it gets a 0.5 boost to its shield production. Again,
the value of the mining bonus does not matter. However, if a square can be
both irrigated and mined, the mining bonus does NOT apply. (i.e. irrigation 
bonus overrides the mining bonus).

If a square is a grassland square without a shield, its shield production is 0
regardless of what's in RULES.TXT.

If a square is a grassland square with a shield, its shield production is 1 
regardless of what's in RULES.TXT.

Note that MapCopy does not read any RULES.TXT files, but has the default values
from the original Civ2 RULES.TXT hardcoded in.


Future Ideas 

Below are ideas for future improvements to MapCopy, or for future Civ 2
related utilities. Let me know if you like these ideas.

1.  Add the ability to copy units and cities to MapCopy.

2.  Add the ability to specify a range of squares to copy between maps.
    aka -sr1:1:20:20 -dr31:31:40:40 would copy squares ranging from 1,1
    to 20,20 (in Civ2 coordinates) to the squares 31,31 to 40,40 on the
    destination map.

3.  Add the ability to read a RULES.TXT file for terrain production values
    used when calculating fertility.

3.  I've considered a command line utility for scenario writers that would
    accept a script file for altering saved games. For example, the following
    script might cut every Roman citiy's population in half.

    foreach city do
        if city.owner = Romans then
            city.population = city.population / 2
        endif
    endfor

4.  Of course, I could always write The One True Civ2 Scenario Editor, which
    would provide a nice GUI for editing all features of maps, cities, units, 
    events, etc.  Of course, by the time it gets finished Civ 10 will be out.

Reporting bugs

  I've tested MapCopy as much as I can, but there are probably some bugs
  lurking somewhere in the code. If you find any bugs or have any
  requests, you may send me e-mail at jreichwe@san.rr.com, but I can't 
  guarantee that I'll have time to fix anything.

  If you do find a bug in MapCopy, I need detailed feedback. For example:

   * Indicate which version of Civ 2 you are working with (MGE, or FW, ToT). 
   * Indicate what operating system you are working on.
   * A detailed description of how to reproduce the problem.  
     If possible include any .MP or .SAV files that are causing problems.
   
Fertility Tools

I spent way, way too much time trying to figure out how Civ 2 calculates
fertility. In the end I did not have quite enough time to fully do so, but in
the process I wrote a couple of tools based on MapCopy to help me out what
Civ 2 is doing. I have included these tools in case someone else out there is 
curious about how fertility is calculated. These are:

getfert.exe    Returns the fertility value of a single square. The
               squares coordinates are in Civ 2 coordinates, not the 
               map editor's coordinates. Example:

               getfert ca_b4000.sav 45 47

FertDiff.exe   Compares the fertility of two equally sized maps. If there
               are any differences, prints out the Civ 2 coordinates of the
               suqare that differs, the fertility of file 1, and the fertility
               of file 2. Example:

               fertdiff first.sav second.sav


Working with the Source Code

  MapCopy is open source software. This means that the source code for MapCopy
  is included in this distribution. You can compile MapCopy for yourself, or 
  use, modify, or distribute its source code as stated by the Mozilla Public
  License (see LICENSE.TXT).

  You may use these files for you own projects, as stated in the MPL. If you 
  make an improvement to any of the MapCopy source code, I would appreciate it 
  if you would send the new code to me, so I could incorporate it into future 
  versions.

  Building MapCopy

    MapCopy is written to be fully ANSI C++ compliant. Unfortunately, that isn't
    enough to make a program portable to different compilers. This version
    of MapCopy was compiled using the Dev-C++4.9.9.1 IDE, which uses the MinGW
    compiler. See http://www.bloodshed.net/ for information on Dev-C++ and 
    http://www.mingw.org/ for information on MinGW.

Good Luck, and I hope you find this software useful!
