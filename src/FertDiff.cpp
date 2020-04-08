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
 * The Original Code is FertDiff, a fertiltiy diff utility for Civ2 maps and saved games.
 * 
 * The Initial Developer of the Original Code is James Dustin Reichwein.
 * Portions created by James Dustin Reichwein are
 * Copyright (C) 2005, James Dustin Reichwein.  All
 * Rights Reserved.
 * 
 * Contributor(s): 
 */
 
#include <iostream>
#include <fstream>
#include <string>
#include "DustyUtil.h"
#include "civ2sav.h"


const char *versionText[] =
{
    "FertDiff Version 1.1",
    "Written By James Dustin Reichwein, Copyright (C) 2005. All Rights Reserved.",
    NULL
};
const char *helpText[] =
{
//  "12345678901234567890123456789012345678901234567890123456789012345678901234567890
    "fertdiff file1 file2",
    "  Compares the fertility of two Civ2Saved game files, and displays any", 
    "  differences.",
    NULL
};



namespace
{
    // The files to diff
    string file1;
    string file2;

}

int parseFileNames(int argc, char *argv[]);
void printText(const char *text[]);
void printErrorMessage(const string message);

int main(int argc, char *argv[])
{
    Civ2SavedGame savedGameOne;
    Civ2SavedGame savedGameTwo;

    try
    {
        // Setup default values for command line parameters, parse them,
        // and check for their validity. 
        parseFileNames(argc, argv);
        
        if (file1.empty() || file2.empty())
        {
           printErrorMessage("Must specify two file names on command line.");        
           return 1;
        }
        LogOutput::setOutputStream(cout);
        LogOutput::enableLevel(NORMAL);

        // Open the files
        savedGameOne.load(file1);
        savedGameTwo.load(file2);

        Civ2Map& map1 = savedGameOne.getMap(0);
        Civ2Map& map2 = savedGameTwo.getMap(0);

        int numDiffs = 0;

        // Iterate through map squares, diffing fertility
        // Note that due to the nature of Civ2 maps, not every combination
        // of X and Y is valid.  Specifically, x+y must be even.
        // Also note that this is going through the coordinate system as
        // seen in Civ2, not in the MapEditor
        for (int y = 0; y < savedGameOne.getHeight(); y++)
        {
            for (int x = y % 2; x < savedGameTwo.getWidth(); x+=2)
            {
                if (map1.getFertility(x,y) != map2.getFertility(x,y))
                {
                    numDiffs++;
                    cout << x << "," << y << " " << int(map1.getFertility(x,y)) << "," << int(map2.getFertility(x,y)) << endl;
                }
            }
        }
        cout << numDiffs << " differences found." << endl;
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


// Parses the first two command line arguments, the files to diff.
// The return value is the index of the next unparsed parameter. 
int parseFileNames(int argc, char *argv[])
{
    // If too few parameters, singal main to print help text
    if (argc < 2) return 1;

    // The first argument should be the source file name
    if (argv[1] != NULL)
    {
        file1  = argv[1];
    }

    // The second argument should be the destination file name
    if (argc >=3 && argv[2] != NULL)
    {
        file2 = argv[2];
    }
    else return 2;

    return 3;
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

