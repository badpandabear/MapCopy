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
 * Copyright (C) 2000,2005, James Dustin Reichwein.  All
 * Rights Reserved.
 * 
 * Contributor(s): 
 */
 
// getfert.cpp stolen from mapcopy to extract fertility information
// Description:  A utility for copying Civ2 maps and saved games.
//
// Creation Date: Aug/13/2000
//
// Created By: Dusty Reichwein <jreichwe@san.rr.com>
//
// Revision History:
// Sep/13/2000  JDR  Initial 1.Beta1 version.
// Feb/10/2005  JDR  Copied from mapcopy.cpp to make getfert.cpp

#include <iostream>
#include <fstream>
#include <string>
#include "DustyUtil.h"
#include "civ2sav.h"


void printErrorMessage(const string message);


int main(int argc, char *argv[])
{
    Civ2SavedGame one;
    Civ2SavedGame two;
    if (argc != 4) 
    {
       cout << "Need three and only three arguments.\n<file> <x> <y> (in civ2 coords)";
       return 1;
    }
    string sourceFile = argv[1];
    string xStr = argv[2];
    string yStr = argv[3];

    int x = atoi(xStr.c_str());
    int y = atoi(yStr.c_str());

    try
    {
        
        // This tells the Civ2SavedGame objects whether to print detailed messages
        LogOutput::setOutputStream(cout);
        LogOutput::enableLevel(NORMAL);


        one.load(sourceFile);


        if (x < 0 || x > one.getWidth())
        {
            LogOutput::log(NORMAL) << "X Value: " << x << " is invalid\n";
            return 1;
        }

        if (y < 0 || y > one.getHeight())
        {
            LogOutput::log(NORMAL) << "Y Value: " << y << " is invalid\n";
            return 1;
        }

        LogOutput::log(NORMAL) << hex << "Fertility is: " << (unsigned int)(one.getMap(0).getFertility(x,y)) << endl;
    }
    catch(exception& e)
    {
        printErrorMessage(e.what());
        return 1;
    }
    catch(...)
    {
        cout << "Unknown error!\n";
        return 1;
    }
    return 0;
}
// Displays an error message
void printErrorMessage(string message)
{
    cout << message << endl;
}

