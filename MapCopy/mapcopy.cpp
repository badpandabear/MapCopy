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

#include <fstream>
#include <string>
#include <DustyUtil.h>
#include "civ2sav.h"


const char *versionText[] =
{
    "MapCopy Version 1.Beta1",
    "Written By James Dustin Reichwein, Copyright (C) 2000. All Rights Reserved.",
    NULL
};
const char *helpText[] =
{
//  "12345678901234567890123456789012345678901234567890123456789012345678901234567890
    "mapcopy [source] dest [ options ]",
    "  Copies the Civ2 map from file \"source\" to file \"dest\".",
    "  See readme.txt for more information.",
    "  Options: (+x turns option x on. -x turns option x off.) ",
    "    s[eed]        Copies the resource seed.",
    "    t[errain]     Copies the terrain data.",
    "    i[mprovement] Copies terrain improvements.",
    "    v[isibility]  Copies terrain visibility.",
    "    o[wnership]   Copies terrain owner ship.",
    "    cs            Copies civilization start locations from an MP file.",
    "    bc            Copies \"body counter\" values for continents.",
    "    cr            Copies \"city radius\" data for terrain.",
    "    v[erbose]     Enables informative screen messages.",
    "    b[ackup]      Creates of a backup named \"dest.bak\". (on by default)",
    "    /? or -h      Displays this help screen.",
    "    f[ertility][:CALC|CALCALL|ADJUST|ZERO]",
    "                  Copies or calculates fertility data for terrain. ",
    "    cv[:CURRENT]  Copies civ specific visible terrain improvement data.",
    NULL
};



namespace
{
    enum OPTIONS { SEED = 0, TERRAIN, IMPROVEMENT, VISIBILITY, OWNERSHIP,
                   CIV_START, BODY_COUNTER, CITY_RADIUS, VERBOSE, BACKUP,
                   FERTILITY, CIV_VIEW, NUM_OPTIONS };
    enum OP_VALUE { OFF=0, ON=1, COPY=1, CALC, CALCALL, ADJUST, CURRENT, ZERO };

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
        /* civ_view */    { OFF,     OFF,       OFF,      OFF,     OFF, OFF} 
    };


    // The source and destination files
    string sourceFile = "";
    string destFile = "";

    // The type of copy
    COPYTYPE copy_type = MP2MP;

    // The size used for the buffer used when backing up
    unsigned int BACKUP_BUFFER_SIZE = 1024;
}

void setDefaults();
void parseCommandLine(int argc, char *argv[]);
int parseFileNames(int argc, char *argv[]);
void parseOptions(int i, int argc, char *argv[]);
void checkArgumentValidity();
void printText(const char *text[]);
void printErrorMessage(const string message);
void backupFile(string file) throw (runtime_error);

int main(int argc, char *argv[])
{
    Civ2SavedGame one;
    Civ2SavedGame two;


    try
    {
        // Setup default values for command line parameters, parse them,
        // and check for their validity. 
        parseCommandLine(argc, argv);
        
        // This tells the Civ2SavedGame objects whether to print detailed messages
        one.setVerbose(options[VERBOSE] == ON ? true : false);
        two.setVerbose(options[VERBOSE] == ON ? true : false);

        if (options[BACKUP] == ON) backupFile(destFile);

        // Load a source file if one is provided
        if (copy_type != MP && copy_type != SAV) one.load(sourceFile);

        if (fileExists(destFile))
        {
            two.load(destFile);
        }
        else if (Civ2SavedGame::isMPFile(destFile))
        {
            two.createMP(one.getWidth(), one.getHeight());
        }
        else
        {
            two.createSAV(one.getWidth(), one.getHeight());
        }

        // Confirm maps are the same size.
        if (copy_type != MP && copy_type != SAV)
        {
            if (one.getWidth() != two.getWidth() ||
                one.getHeight() != two.getHeight())
            {
                throw runtime_error("Both maps must be the same size!");
            }
        }

        // Copy resource seed.
        if (options[SEED]==COPY) two.setSeed(one.getSeed());

        if (options[CIV_START] == COPY)
        {
            two.setCivStart(one.getCivStart());
        }

        // Iterate through map squares, copying info
        // Note that due to the nature of Civ2 maps, not every combination
        // of X and Y is valid.  Specifically, x+y must be even.
        // Also note that this is going through the coordinate system as
        // seen in Civ2, not in the MapEditor
        for (int y = 0; y < two.getHeight(); y++)
        {
            for (int x = y % 2; x < two.getWidth(); x+=2)
            {
                // Terrain includes terrain type, the river flag, and
                // Whether resources are hidden
                if (options[TERRAIN]==COPY)
                {
                    two.setRiver(x, y, one.isRiver(x, y));
                    two.setResourceHidden(x, y, one.isResourceHidden(x, y));
                    two.setTypeIndex(x, y, one.getTypeIndex(x, y));
                }
                if (options[IMPROVEMENT] == COPY)
                {
                    two.setImprovements(x, y, one.getImprovements(x, y));
                }
                // This governs what civs see what squares
                if (options[VISIBILITY] == COPY)
                {
                    two.setVisibility(x, y, one.getVisibility(x, y));
                }
                if (options[OWNERSHIP] == COPY)
                {
                    two.setOwnership(x, y, one.getOwnership(x, y));
                }

                // The body_counter is a # assigned to a continent. It
                // can be calculated by the map editor by doing an analyze map
                if (options[BODY_COUNTER] == COPY)
                {
                    two.setBodyCounter(x, y, one.getBodyCounter(x, y));
                }
                if (options[CITY_RADIUS] == COPY)
                {
                    two.setCityRadius(x, y, one.getCityRadius(x, y));
                }

                // Fertility is tricky.
                switch (options[FERTILITY])
                {
                    case COPY:
                        two.setFertility(x, y, one.getFertility(x, y));
                        break;

                    case CALC:
                        if (two.getTypeIndex(x, y) == Civ2SavedGame::GRASSLAND ||
                            two.getTypeIndex(x, y) == Civ2SavedGame::PLAINS)
                        {
                            two.calcFertility(x, y);
                        }
                        else two.setFertility(x, y, 0);
                        break;

                    case CALCALL:
                        if (two.getTypeIndex(x, y) != Civ2SavedGame::OCEAN)
                        {
                            two.calcFertility(x, y);
                        }
                        else two.setFertility(x, y, 0);
                        break;

                    case ADJUST:                      
                        if (two.getTypeIndex(x, y) != Civ2SavedGame::OCEAN)
                        {
                            two.setFertility(x, y, one.getFertility(x, y));
                            two.adjustFertility(x, y);
                        }
                        else two.setFertility(x, y, 0);
                        break;

                    case ZERO:
                        two.setFertility(x, y, 0);
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
                        two.setCivView(x, y, Civ2SavedGame::ALL,
                                       two.getImprovements(x, y));
                        break;

                    case COPY:
                        two.setCivView(x, y, Civ2SavedGame::WHITE,
                                       one.getCivView(x, y, Civ2SavedGame::WHITE));
                        two.setCivView(x, y, Civ2SavedGame::GREEN,
                                       one.getCivView(x, y, Civ2SavedGame::GREEN));
                        two.setCivView(x, y, Civ2SavedGame::BLUE,
                                       one.getCivView(x, y, Civ2SavedGame::BLUE));
                        two.setCivView(x, y, Civ2SavedGame::YELLOW,
                                       one.getCivView(x, y, Civ2SavedGame::YELLOW));
                        two.setCivView(x, y, Civ2SavedGame::CYAN,
                                       one.getCivView(x, y, Civ2SavedGame::CYAN));
                        two.setCivView(x, y, Civ2SavedGame::ORANGE,
                                       one.getCivView(x, y, Civ2SavedGame::ORANGE));
                        two.setCivView(x, y, Civ2SavedGame::PURPLE,
                                       one.getCivView(x, y, Civ2SavedGame::PURPLE));
                        break;

                    default: // Assume off
                        break;
                }
            } // end inner for
        } // end outer for

        two.save(destFile);
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



// Parses the first two command line arguments, the sourcd and
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
        else if ( o == "verb" || o == "verbose")
        {
            options[VERBOSE] = value;
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
        bool error = false;
        if (options[SEED] == COPY) error = true;
        if (options[TERRAIN] == COPY) error = true;
        if (options[IMPROVEMENT] == COPY) error = true;
        if (options[VISIBILITY] == COPY) error = true;
        if (options[OWNERSHIP] == COPY) error = true;
        if (options[CIV_START] == COPY) error = true;
        if (options[BODY_COUNTER] == COPY) error = true;
        if (options[CITY_RADIUS] == COPY) error = true;
        if (options[FERTILITY] == COPY) error = true;
        if (options[FERTILITY] == ADJUST) error = true;
        if (options[CIV_VIEW] == COPY) error = true;
        
        if (error) throw runtime_error("Cannot copy information when only one file name is given!");
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
    theFile.open(file.c_str(), ios::binary | ios::nocreate);

    if (!theFile)
    {
        // We assume the file doesn't exist, so can't be backed up
        return;
    }

    if (options[VERBOSE] == ON)
    {
        cout << "Backing up." << endl;
    }

    unsigned char *backup_buffer = new unsigned char[BACKUP_BUFFER_SIZE];

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


