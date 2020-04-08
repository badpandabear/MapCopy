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
 * The Original Code is DustyUtil, a set of utility functions and classes.
 * 
 * The Initial Developer of the Original Code is James Dustin Reichwein.
 * Portions created by James Dustin Reichwein are
 * Copyright (C) 2000, James Dustin Reichwein.  All
 * Rights Reserved.
 * 
 * Contributor(s): 
 */


////////////////////////////////////////////////////////////////////////////////
// Source File: DustyUtil.cpp
//
// Description: Dusty's utility functions
//
// Written By:  Dusty Reichwein
//
// Version:     1.0
//
// Created:     December 8, 1997
//
// Notes:       This file is mainly for usefull functions that aren't
//              standard.  
//
// Modification History:
// Dec/08/1997  JDR  Last version for LDisk project.
// Sep/03/2000  JDR  Copied for generic utilitiy 1.0
//              JDR  Added additional functions, removed borland specific strncmpi
// Sep/13/2000  JDR  Released under MPL
// Feb/10/2005  JDR  Removed non-standard open mode from fileExists()
// Feb/13/2005  JDR  Added LogOutput class
// May/22/2005  JDR  Added support for multiple log levels.
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <ctype.h>
#include <fstream>

#include "DustyUtil.h"

using namespace std;

// Convert strings to lower case
string DustyUtil::convert_to_lower(string& s)
{
    for (unsigned int i = 0; i < s.length(); i++)
    {
        s.replace(i, 1, 1, tolower(s[i]));
    }
    return s;

}
string DustyUtil::convert_to_upper(string& s)
{
    for (unsigned int i = 0; i < s.length(); i++)
    {
        s.replace(i, 1, 1, toupper(s[i]));
    }
    return s;
}

// Create a copy of the string of a new case
string DustyUtil::copy_to_lower(const string s)
{
    string copy(s);

    for (unsigned int i = 0; i < s.length(); i++)
    {
        copy.replace(i, 1, 1, tolower(s[i]));
    }
    return copy;
}

string DustyUtil::copy_to_upper(const string s)
{
    string copy(s);

    for (unsigned int i = 0; i < s.length(); i++)
    {
        copy.replace(i, 1, 1, toupper(s[i]));
    }
    return copy;
}

// Determines if a file exists
bool DustyUtil::fileExists(const string filename)
{
    ifstream i;
    i.open(filename.c_str());
    if (i) return true;
    else return false;
    // Note i's destructor will close file
}

//////////////// Log output //////////////////////////////////////////////
ostream* DustyUtil::LogOutput::stream = NULL;
vector<bool> DustyUtil::LogOutput::enabled;
ostream DustyUtil::LogOutput::nullStream(new nullBuf());
