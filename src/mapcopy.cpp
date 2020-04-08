/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MapCopy, a copy utility for Civ2 maps and saved games.
 * 
 * The Initial Developer of the Original Code is James Dustin Reichwein.
 * Portions created by James Dustin Reichwein are
 * Copyright (C) 2000, James Dustin Reichwein.  All
 * Rights Reserved.
 * 
 * Contributor(s): 
 */
 
// mapcopy.cpp
// Description:  A utility for copying Civ2 maps and saved games.
//
// Creation Date: Aug/13/2000
//
// Created By: Dusty Reichwein <jreichwe@san.rr.com>
//
// Revision History:
// Sep/13/2000  JDR  Initial 1.Beta1 version.
// Feb/10/2005  JDR  Changes for compatability with GCC based mingw compiler.
// Feb/13/2005  JDR  Added options for resource supression
// Feb/20/2005  JDR  1.2Beta1 release of ToT multi-map support
// Jul/02/2005  JDR  1.2 final version.
#include <iostream>
#include <fstream>
#include <string>
#include "DustyUtil.h"
#include "civ2sav.h"


const char *versionText[] =
{
    "MapCopy Version 1.2",
    "Written By James Dustin Reichwein, Copyright (C) 2000,2005. All Rights Reserved.",
    NULL
};
const char *helpText[] =
{
//  "12345678901234567890123456789012345678901234567890123456789012345678901234567890
    "mapcopy [source] dest [ options ]",
    "  Copies the Civ2 map from file \"source\" to file \"dest\".",
    "  See readme.txt for more information.",
    "  Options: (+x turns option x on. -x turns option x off.) ",
    "    s[eed]          Copies the resource seed.",
    "    t[errain]       Copies the terrain data.",
    "    i[mprovement]   Copies terrain improvements.",
    "    v[isibility]    Copies terrain visibility.",
    "    o[wnership]     Copies terrain owner ship.",
    "    rs[:SET|:CLEAR] Copies or sets resources supression.",
    "    cs              Copies civilization start locations from an MP file.",
    "    bc              Copies \"body counter\" values for continents.",
    "    cr              Copies \"city radius\" data for terrain.",
    "    verb[ose][:DEV] Enables informative screen messages. Using 'DEV' results",
    "                    in a very verbose output meant for debugging mapcopy.",
    "    b[ackup]        Creates of a backup named \"dest.bak\". (on by default)",
    "    /? or -h        Displays this help screen.",
    "    f[ertility][:CALC|CALCALL|ADJUST|ZERO]",
    "                    Copies or calculates fertility data for terrain. ",
    "    cv[:CURRENT]    Copies civ specific visible terrain improvement data.",
    "    sm:n or sm:ALL  Picks which map in a multi-map ToT file to copy from.",
    "    dm:n or dm:ALL  Picks which map in a multi-map ToT file to copy to.",
    NULL
};



namespace
{
    enum OPTIONS { SEED = 0, TERRAIN, IMPROVEMENT, VISIBILITY, OWNERSHIP,
                   CIV_START, BODY_COUNTER, CITY_RADIUS, VERBOSE, BACKUP,
                   FERTILITY, CIV_VIEW, RESOURCE_SUP, NUM_OPTIONS };
    enum OP_VALUE { OFF=0, ON=1, COPY=1, CALC, CALCALL, ADJUST, CURRENT, ZERO,
                    SET, CLEAR, DEV };

    // The options
    OP_VALUE options[NUM_OPTIONS];

    // Possible file type configurations
    enum COPYTYPE { MP2MP=0, SAV2SAV, MP2SAV, SAV2MP, MP, SAV, NUM_TYPES };

    // Default option values
    OP_VALUE defaults[NUM_OPTIONS][NUM_TYPES] =
    {
        //                  MP->MP   SAV->SAV   MP->SAV   SAV->MP, MP,  SAV
        /* seed */        { COPY,    COPY,      COPY,     COPY,    OFF, OFF},
        /* terrain */     { COPY,    COPY,      COPY,     COPY,    OFF, OFF},
        /* improvement */ { COPY,    OFF,       OFF,      OFF,     OFF, OFF},
        /* visibility */  { COPY,    OFF,       OFF,      OFF,     OFF, OFF},
        /* owenership */  { COPY,    OFF,       OFF,      OFF,     OFF, OFF},
        /* civ_start  */  { COPY,    OFF,       OFF,      OFF,     OFF, OFF},
        /* body_counter */{ COPY,    COPY,      COPY,     COPY,    OFF, OFF},
        /* city_radius */ { COPY,    OFF,       OFF,      OFF,     OFF, OFF},
        /* verbose */     { ON,      ON,        ON,       ON,      ON,  ON}, 
        /* backup */      { ON,      ON,        ON,       ON,      ON,  ON}, 
        /* fertility */   { COPY,    ADJUST,    CALC,     OFF,     OFF, OFF},
        /* civ_view */    { OFF,     OFF,       OFF,      OFF,     OFF, OFF},
        /* resource 
           supression */  { COPY,    COPY,      COPY,     COPY,    OFF, OFF}
    };


    // The source and destination files
    string sourceFile = "";
    string destFile = "";

    // The source and destination maps for ToT multimap saved games.
    // Note 0 equals "all", -1 equals "default"
    int sourceMap = -1;
    int destMap = -1;

    // The type of copy
    COPYTYPE copy_type = MP2MP;

    // The size used for the buffer used when backing up
    unsigned int BACKUP_BUFFER_SIZE = 1024;
}

void setDefaults();
void setMapDefaults(bool oneSupprtsMultiMaps, bool twoSupportsMultiMaps);
void parseCommandLine(int argc, char *argv[]);
int parseFileNames(int argc, char *argv[]);
void parseOptions(int i, int argc, char *argv[]);
void checkArgumentValidity();
void printText(const char *text[]);
void printErrorMessage(const string message);
void backupFile(string file) throw (runtime_error);
void doMapCopy(const Civ2Map& source, Civ2Map& dest) throw(runtime_error);
void logFileDetails(const Civ2SavedGame& file);

int main(int argc, char *argv[])
{
    Civ2SavedGame sourceSavedGame;
    Civ2SavedGame destSavedGame;

    // I'm using pointers for one and two so that for an inplace modification
    // I can set the source (one) equal to the destination
    Civ2SavedGame *one = &sourceSavedGame;
    Civ2SavedGame *two = &destSavedGame;

    try
    {
        // Setup default values for command line parameters, parse them,
        // and check for their validity. 
        parseCommandLine(argc, argv);
        
        // This tells the Civ2SavedGame objects whether to print detailed messages
        if (options[VERBOSE]== ON || options[VERBOSE] == DEV)
        {
            LogOutput::setOutputStream(cout);
            LogOutput::enableLevel(NORMAL);
            if (options[VERBOSE] == DEV) LogOutput::enableLevel(DEBUG);
        }
            
        if (options[BACKUP] == ON) backupFile(destFile);

        // Load a source file if one is provided
        if (copy_type != MP && copy_type != SAV) 
        {
            LogOutput::log(NORMAL) << "Loading File: " << sourceFile << endl;
            one->load(sourceFile);
            logFileDetails(*one);
        }
        else // An in-place modification
        {
            one = two;
        }
        if (fileExists(destFile))
        {
            LogOutput::log(NORMAL) << "Loading File: " << destFile << endl;
            two->load(destFile);
            logFileDetails(*two);
        }
        else if (Civ2SavedGame::isMPFile(destFile))
        {
            LogOutput::log(NORMAL) << "Creating MP File: " << destFile << endl;
            two->createMP(one->getWidth(), one->getHeight());
        }
        else
        {
            LogOutput::log(NORMAL) << "Creating SAV File: " << destFile << endl;
            two->createSAV(one->getWidth(), one->getHeight());
        }

        // Confirm maps are the same size.
        if (one->getWidth() != two->getWidth() ||
            one->getHeight() != two->getHeight())
        {
            throw runtime_error("Both maps must be the same size!");
        }

        // Setup default for ToT to ToT copies to be "all"
      
        setMapDefaults(one->supportsMultiMaps(), two->supportsMultiMaps());

        // Check for an ALL copy that would require a non-ToT saved game
        // to support multiple maps
        if (sourceMap == 0 && sourceMap > two->getNumMaps()&& two->supportsMultiMaps() == false)
        {
            throw runtime_error("Cannot create multiple maps in a non-ToT saved game.");
        }

        // Check for a map to map copy that would require a non-ToT saved game
        // to support multiple maps
        if(destMap > 1 && two->supportsMultiMaps() == false)
        {
            throw runtime_error("Cannot create multiple maps in a non-ToT saved game.");
        }

        // Check for a non-existant source map
        if (sourceMap > one->getNumMaps()) 
        {
            throw runtime_error("Map specified with +sm does not exist within source file.");
        }

        // Check for a copy that creates a "gap" in the destination
        if (destMap > two->getNumMaps() + 1) 
        {
            throw runtime_error("Cannot create a gap between maps in a ToT saved game.");
        }

        // Copy main resource seed.
        if (options[SEED]==COPY) two->setSeed(one->getSeed());

        // Copy civilization start positions
        if (options[CIV_START] == COPY)
        {
            two->setCivStart(one->getCivStart());
        }


        // There are three types of copies.

        // First type: Copying one source map into one destination map
        if (sourceMap > 0 && destMap > 0)
        {
            // Note sourceMap and destMap are one based, while the indices
            // used by addMap() and getMap() are zero based.

            if (destMap > two->getNumMaps())
            {
                LogOutput::log(NORMAL) << "Adding map " << destMap << " to " << destFile << endl;

                two->addMap(destMap  - 1); // Add additional map to destination
                                          // if needed
            }

            LogOutput::log(NORMAL) << "Copying map " << sourceMap << " to " << destMap << endl;            

            // Do the actual copy.
            doMapCopy(one->getMap(sourceMap - 1), two->getMap(destMap - 1));

        }
        // Second type: Copying multiple source maps into the destination file
        else if (sourceMap == 0 && destMap == 0)
        {
            for (int i = 0; i < one->getNumMaps(); i++)
            {
                if (i >= two->getNumMaps())
                {
                    LogOutput::log(NORMAL) << "Adding map " << i + 1 << " to " << destFile << endl;

                    two->addMap(i); // Add additional map to destination
                                   // if needed
                }
                LogOutput::log(NORMAL) << "Copying map " << i+1 << " to " << i+1 << endl;            
                doMapCopy(one->getMap(i), two->getMap(i));
            }
        }
        // Copying one source map over all maps in the destination file
        else if (sourceMap != 0 && destMap == 0)
        {
            Civ2Map& source = one->getMap(sourceMap-1);
            for (int i = 0; i < two->getNumMaps(); i++)
            {
                LogOutput::log(NORMAL) << "Copying map " << sourceMap << " to " << i + 1 << endl;  
                doMapCopy(source, two->getMap(i));
            }
        }
        else
        {
            // Really with all the error checking this should never happen.
            // Of course, that means it will probably happen the first
            // time I run the program.
            stringstream message;
            message << "Unrecognized copy type. sm = " << sourceMap << " dm = " << destMap;
            throw runtime_error(message.str());
        }

        // if the destination does not support multiple maps, its main map
        // seed should be set to that of its only map. This is because
        // non-ToT saved game files (and .MPs) don't support a map specific
        // resource seed
        if (two->supportsMultiMaps() == false && options[SEED] == COPY)
        {
            two->setSeed(two->getMap(0).getSeed());
        }
                
        // Save the results into the destination file
        two->save(destFile);
    }
    catch(exception& e)
    {
        printErrorMessage(e.what());
        return 1;
    }
    catch(int& e)
    {
        // int is throw to indicate to print help text
        // 0 == help text, 1 == version text only
        printText(versionText);
        if (e==0) printText(helpText);
        return 0;
    }
    catch(...)
    {
        cout << "Unknown error!\n";
        return 1;
    }
    return 0;
}

// Performs a copy between two Civ2Map objects
void doMapCopy(const Civ2Map& source, Civ2Map& dest) throw(runtime_error)
{
    // Set to true if a second pass through the map is needed
    bool secondPassNeeded = false;

    // Copy map specific resource seed
    if (options[SEED] == COPY) dest.setSeed(source.getSeed());

    // Iterate through map squares, copying info
    // Note that due to the nature of Civ2 maps, not every combination
    // of X and Y is valid.  Specifically, x+y must be even.
    // Also note that this is going through the coordinate system as
    // seen in Civ2, not in the MapEditor
    for (int y = 0; y < dest.getHeight(); y++)
    {
        for (int x = y % 2; x < dest.getWidth(); x+=2)
        {
            // Terrain includes terrain type, and the river flag
            if (options[TERRAIN]==COPY)
            {
                dest.setRiver(x, y, source.isRiver(x, y));
                dest.setTerrainType(x, y, source.getTerrainType(x, y));
            }
            if (options[IMPROVEMENT] == COPY)
            {
                dest.setImprovements(x, y, source.getImprovements(x, y));
            }
            // This governs what civs see what squares
            if (options[VISIBILITY] == COPY)
            {
                dest.setVisibility(x, y, source.getVisibility(x, y));
            }
            if (options[OWNERSHIP] == COPY)
            {
                dest.setOwnership(x, y, source.getOwnership(x, y));
            }

            // The body_counter is a # assigned to a continent. It
            // can be calculated by the map editor by doing an analyze map
            if (options[BODY_COUNTER] == COPY)
            {
                dest.setBodyCounter(x, y, source.getBodyCounter(x, y));
            }

            if (options[CITY_RADIUS] == COPY)
            {
                dest.setCityRadius(x, y, source.getCityRadius(x, y));
            }

            // Fertility is tricky.
            switch (options[FERTILITY])
            {
                case COPY:
                    dest.setFertility(x, y, source.getFertility(x, y));
                    break;

                case CALC:
                    secondPassNeeded = true;
                    break;

                case CALCALL:
                    secondPassNeeded = true;
                    break;

                case ADJUST:                      
                    secondPassNeeded = true;
                    break;

                case ZERO:
                    dest.setFertility(x, y, 0);
                    break;

                default:
                    // Assume off, don't copy
                    break;
            }

            // "civ_view" is the improvement information that each civilization
            // sees.  Each civilization only sees the terrain improvement info
            // that was current when a unit was near that square.  Hence you
            // need to reexplore to see what other civs have been up to.

            // If the civ_view option is CURRENT, each civilization will see
            // the most current improvement information
            switch(options[CIV_VIEW])
            {
                case CURRENT:
                    dest.setCivView(x, y, Civ2Map::ALL,
                                   dest.getImprovements(x, y));
                    break;

                case COPY:
                    dest.setCivView(x, y, Civ2Map::WHITE,
                                   source.getCivView(x, y, Civ2Map::WHITE));
                    dest.setCivView(x, y, Civ2Map::GREEN,
                                   source.getCivView(x, y, Civ2Map::GREEN));
                    dest.setCivView(x, y, Civ2Map::BLUE,
                                   source.getCivView(x, y, Civ2Map::BLUE));
                    dest.setCivView(x, y, Civ2Map::YELLOW,
                                   source.getCivView(x, y, Civ2Map::YELLOW));
                    dest.setCivView(x, y, Civ2Map::CYAN,
                                   source.getCivView(x, y, Civ2Map::CYAN));
                    dest.setCivView(x, y, Civ2Map::ORANGE,
                                   source.getCivView(x, y, Civ2Map::ORANGE));
                    dest.setCivView(x, y, Civ2Map::PURPLE,
                                   source.getCivView(x, y, Civ2Map::PURPLE));
                    break;

                default: // Assume off
                    break;
            } // end switch on civ view

            // The resource supression flag allows a resource
            // that would be there based on the resource seed to be removed
            // Command line arguments allow it to be copied, cleared (making
            // all resources visible), or set (making all resources disappear)
            switch (options[RESOURCE_SUP])
            {
                case COPY:
                    dest.setResourceHidden(x, y, source.isResourceHidden(x, y));
                    break;
                case CLEAR:
                    dest.setResourceHidden(x, y, false);
                    break;
                case SET:
                    dest.setResourceHidden(x, y, true);
                    break;
                default: // assume off
                    break;
            } // end switch on resource supression
        } // end inner for
    } // end outer for

    // Do a second pass for fertility calculations. Since the calculations
    // for a square depend on adjacent suqares, all squares be in their 
    // final state before calculations can be made. Hence a second pass is used.
    if (secondPassNeeded)
    {
        for (int y = 0; y < dest.getHeight(); y++)
        {
            for (int x = y % 2; x < dest.getWidth(); x+=2)
            {
                switch (options[FERTILITY])
                {
                    case CALC:
                        if (dest.getTerrainType(x, y) == GRASSLAND ||
                            dest.getTerrainType(x, y) == PLAINS)
                        {
                            dest.calcFertility(x, y);
                            dest.adjustFertility(x, y);
                        }
                        else dest.setFertility(x, y, 0);
                        break;

                    case CALCALL:
                        if (dest.getTerrainType(x, y) != OCEAN)
                        {
                            dest.calcFertility(x, y);
                            dest.adjustFertility(x, y);
                        }
                        else dest.setFertility(x, y, 0);
                        break;

                    case ADJUST:                      
                        if (dest.getTerrainType(x, y) != OCEAN)
                        {
                            dest.setFertility(x, y, source.getFertility(x, y));
                            dest.adjustFertility(x, y);
                        }
                        else dest.setFertility(x, y, 0);
                        break;
                    default:
                        // No fertility calculations needed. 
                        break;
                } // end switch
            } // end inner for loop
        } // end outer for loop
    } // end check for second pass
}
// end doMapCopy

// Parse the command line arguments, and verify them.
void parseCommandLine(int argc, char *argv[])
{
    // First parse the file names on the command line
    // Note that there may be one or two files.
    int i = parseFileNames(argc, argv);

    // The filenames determine what default values to use
    setDefaults();

    // Now parse the command line options and verify them
    parseOptions(i, argc, argv);
    checkArgumentValidity();
}



// Parses the first two command line arguments, the source and
// destination file names. The return value is the index of the next
// unparsed parameter.  (2 or 3, depending on whether there are one
// or two file names)
int parseFileNames(int argc, char *argv[])
{
    // If too few parameters, singal main to print help text
    if (argc < 2)
    {
        throw int(0);
    }

    // The first argument should be the source file name
    if (argv[1] != NULL)
    {
        sourceFile = argv[1];
    }
    else
    {
        // We must have at least one file name
        throw runtime_error("Invalid source file name.");
    }

    // The second argument should be the destination file name
    if (argv[2] != NULL && *(argv[2]) != '-' && *(argv[2]) != '+')
    {
        destFile = argv[2];
    }
    else
    {
        // If there isn't a second filename, that means we're doing an inplace
        // modification
        destFile = sourceFile;
        if (Civ2SavedGame::isMPFile(destFile)) copy_type = MP;
        else copy_type = SAV;
        return 2;
    }

    // Otherwise we are doing a file to file copy

    // Figure out the type of copy
    if (Civ2SavedGame::isMPFile(sourceFile))
    {
        if (Civ2SavedGame::isMPFile(destFile)) copy_type = MP2MP;
        else copy_type = MP2SAV;
    }
    else
    {
        if (Civ2SavedGame::isMPFile(destFile)) copy_type = SAV2MP;
        else copy_type = SAV2SAV;
    }

    return 3;
}

// Sets the default option values based on the type of copy being performed
void setDefaults()
{
    for (int i = 0; i<NUM_OPTIONS; i++)
    {
        options[i] = defaults[i][copy_type];
    }
}

// Set default values for sourceMap (+sm) and destMap (+dm) based on whether
// the source and destination accept multiple maps
void setMapDefaults(bool oneSupportsMultiMaps, bool twoSupportsMultiMaps)
{
    // An ALL to ALL copy is default if both maps support multiple maps,
    // and the source and destination have not been set to a specific map
    if (oneSupportsMultiMaps && twoSupportsMultiMaps)
    {
        if ( (sourceMap == -1 || sourceMap == 0) &&
             (destMap == -1 || destMap == 0) )
        {
            sourceMap = 0;
            destMap = 0;
            return;
        }
    }
    // In all other cases, these values default to 1
    if (sourceMap == -1) sourceMap = 1;
    if (destMap == -1) destMap = 1;
}

// Parse the command line options, starting from index
void parseOptions(int index, int argc, char *argv[]) 
{

    // Now we loop through options
    for (int i = index; i < argc; i++)
    {
        // Grab whether + is being used to turn an option on, or if - is
        // being used to turn an option off. The on/off information is stored
        // in value
        OP_VALUE value;

        if (argv[i] != NULL && strlen(argv[i]) >= 2)
        {
            if (*(argv[i]) == '-') value = OFF;
            else if (*(argv[i]) == '+') value = ON;
            else
            {
                throw runtime_error(string("Invalid command line option:") + argv[i]);
            }
        }
        else
        {
            throw runtime_error("Invalid command line option.");
        }

        string o = argv[i] + 1;
        convert_to_lower(o);

        if ( o == "s" || o == "seed")
        {
            options[SEED] = value;
        }
        else if ( o == "t" || o == "terrain")
        {
            options[TERRAIN] = value;
        }
        else if ( o == "i" || o == "improvement")
        {
            options[IMPROVEMENT] = value;
        }
        else if ( o == "v" || o == "visibility")
        {
            options[VISIBILITY] = value;
        }
        else if ( o == "o" || o == "ownership")
        {
            options[OWNERSHIP] = value;
        }
        else if ( o == "cs")
        {
            options[CIV_START] = value;
        }
        else if ( o == "bc") 
        {
            options[BODY_COUNTER] = value;
        }
        else if ( o == "cr") 
        {
            options[CITY_RADIUS] = value;
        }
        else if ( o.compare(0,4, "verb")==0 )
        {
            int colon = o.find_first_of(':');
            if (colon != string::npos && o.compare(colon, string::npos, ":dev") == 0) options[VERBOSE]=DEV;
            else options[VERBOSE] = value;
        }
        else if ( o == "b" || o == "backup")
        {
            options[BACKUP] = value;
        }
        else if (o == "cv")
        {
            options[CIV_VIEW] = value;
        }
        else if (o == "cv:current")
        {
            options[CIV_VIEW] = CURRENT;
        }
        else if ( o == "f" || o == "fertility")
        {
            options[FERTILITY] = value;
        }
        else if ( o.compare(0, 2, "f:") == 0 ||
                  o.compare(0, 10, "fertility:") == 0 )
        {
            int colon = o.find_first_of(':');
            if (o.compare(colon, string::npos, ":calc") == 0)    options[FERTILITY] = CALC;
            else if (o.compare(colon, string::npos, ":calcall") == 0) options[FERTILITY] = CALCALL;
            else if (o.compare(colon, string::npos, ":adjust") == 0)  options[FERTILITY] = ADJUST;
            else if (o.compare(colon, string::npos, ":zero") == 0)    options[FERTILITY] = ZERO;
            else
            {
                throw runtime_error("Unknown Option: " + o);
            }
        }
        else if ( o == "rs")
        {
            options[RESOURCE_SUP] = value;
        }
        else if ( o == "rs:set" )
        {
            options[RESOURCE_SUP] = SET;
        }
        else if ( o == "rs:clear" )
        {
            options[RESOURCE_SUP] = CLEAR;
        }
        else if (o.compare(0, 3, "sm:") == 0 || 
                 o.compare(0, 3, "dm:") == 0 )
        {
            int map_num = 0; // 0 is used for "all"
            if (o.compare(2, string::npos, ":all") != 0)
            {
                // The value is not "all", convert to integer
                if (o.size() <4)
                {
                    throw runtime_error("Missing map # for option " + o);
                }
                map_num = atoi(o.substr(3, string::npos).c_str());

                if (map_num <1 || map_num > 4) 
                {
                    throw runtime_error("Invalid map # for option " + o);
                }
            }
            if (o.compare(0, 3, "sm:") == 0) sourceMap = map_num;
            else destMap = map_num;        
        }
        else
        {
            throw runtime_error("Unknown Option: " +  o);
        }
    } // end for

}
// end parseOptions

// Determines if the argument settings are valid for the current copy type
void checkArgumentValidity() 
{

    // Check to see that files are specified
    if (copy_type == MP || copy_type == SAV)
    {
        if (destFile.length()==0) throw int(0);
    }
    else if (sourceFile.length() == 0 || destFile.length() == 0)
    {
        throw int(0);
    }

    // Validate multimap copies
    if (sourceMap > 1) 
    {
        if (copy_type == MP || copy_type == MP2MP || copy_type == MP2SAV)
        {
            throw runtime_error("Invalid source map option: MP files only have one map.");
        }
    }
    if (destMap != 1 && destMap != -1)
    {
        if (copy_type == MP || copy_type == MP2MP || copy_type == SAV2MP)
        {
            throw runtime_error("Invalid dest map option: MP files only have one map.");
        }
    }
    if (sourceMap == 0 && destMap != 0 && destMap != -1)
    {
        throw runtime_error("Invalid source map option: Cannot copy 'ALL' maps to a single map.");
    }
     

    // Copying start positions is only valid for MP2MP copies
    if (options[CIV_START] != OFF && copy_type != MP2MP)
    {
        throw runtime_error("Invalid cs option: Can only copy starting positions between .MP files!");
    }

    // Copying Civ View information is only acceptable for SAV2SAV copies,
    // but it is possible to calculate the Civ View information if the
    // destination is a SAV file.
    if (options[CIV_VIEW] == COPY && copy_type != SAV2SAV)
    {
        throw runtime_error("Invalid cv option: Can only copy civ view data between .SAV files!");
    }

    if (options[CIV_VIEW] == CURRENT && copy_type != MP2SAV
                                     && copy_type != SAV2SAV
                                     && copy_type != SAV)
    {
        throw runtime_error("Invalid cs option: Can only calculate civ view data for .SAV destiantion files!");
    }

    if (copy_type == MP || copy_type == SAV)
    {
        // Make sure destination file exists
        if (!DustyUtil::fileExists(destFile))
        {
            throw runtime_error(string("File ") + destFile + " must exist for in place modification.");
        }
    }
}

// Display information about a saved game information
void logFileDetails(const Civ2SavedGame& file)
{
    int width = file.getWidth();
    int height = file.getHeight();
    
    if (file.isMapOnly())
    {
        LogOutput::log(NORMAL) << width << " by " << height << " MP file." << endl;
    }
    else
    {
        string shape = " round ";
        if (file.isFlatEarth()) shape = " flat ";

        LogOutput::log(NORMAL) << width << " by " << height << " " 
                               << file.getVersionString() << shape << "earth SAV/SCN file with " 
                               << file.getNumMaps() << " maps." << endl;
    }
}
// Displays an array of strings, one line at a time. Stops when it hits a
// NULL string
void printText(const char *text[])
{
    if (text == NULL)
    {
        cout << "Internal Error: printText called on NULL!";
    }

    for (int i =0; text[i] != NULL; i++)
    {
        cout << text[i] << endl;
    }
}

// Displays an error message
void printErrorMessage(string message)
{
    cout << message << endl;
    cout << "Type \"mapcopy /?\" or see the readme.txt file for help.\n";
}

void backupFile(string file) throw (runtime_error)
{
    ifstream theFile;
    theFile.open(file.c_str(), ios::binary | ios::in);

    if (!theFile)
    {
        // We assume the file doesn't exist, so can't be backed up
        return;
    }

    LogOutput::log(NORMAL) << "Backing up '" << file << "'." << endl;

    char *backup_buffer = new char[BACKUP_BUFFER_SIZE];

    if (backup_buffer == NULL)
    {
        throw runtime_error("Insufficient memory for backup.");
    }

    string backupName = file + ".bak";

    ofstream backupFile;

    backupFile.open(backupName.c_str(), ios::binary);

    if (!backupFile)
    {
        throw runtime_error(string("Error backing up file: ") + file +
                            string(" to file: " + backupName));
    }

    while (theFile && backupFile)
    {
        theFile.read(backup_buffer, BACKUP_BUFFER_SIZE);
        backupFile.write(backup_buffer, theFile.gcount());
    }

    delete[] backup_buffer;

    if (!theFile.eof() || !backupFile)
    {
        throw runtime_error(string("Error backing up file: ") + file +
                            string(" to file: " + backupName));
    }
}


