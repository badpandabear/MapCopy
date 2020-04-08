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
 * The Original Code is civ2sav, a utility class for accessing Civ2 saved games.
 * 
 * The Initial Developer of the Original Code is James Dustin Reichwein.
 * Portions created by James Dustin Reichwein are
 * Copyright (C) 2000, James Dustin Reichwein.  All
 * Rights Reserved.
 * 
 * Contributor(s):
 * Jorrit "Mercator" Vermeiren
 */

// civ2sav.cpp
// Description:  Contains class for reading Civ 2 saved game files.
//
// Creation Date: Aug/13/2000
//
// Created By: Dusty Reichwein
//
// Revision History:
// Sep/13/2000 JDR  Initial 1.Beta1 version.
// Aug/27/2004 JMV  Added support for pre-FW and ToT Civ2 saved game files.
//                  The only restriction now is that the secondary maps in
//                  multi-map ToT games can not be accessed.
//                  Code changes marked with "MERCATOR".
// Feb/10/2005 JDR  Changes for mingw compiler. Specifically, added new classes
//                  Improvements and WhichCivs to avoid using non-portable 
//                  bitfield members. Changed fertility/ownership to one
//                  field to avoid using bitfields. Made fstream open
//                  parameters comply with C++ standard.
// Feb/20/2005 JDR  1.2Beta1 release of ToT multi-map support

#include <iostream>

#include "civ2sav.h"

/////////////////////// Improvements Constants ///////////////////////////////
const unsigned char Improvements::POLLUTION_MASK  = 0x80;
const unsigned char Improvements::FORTRESS_MASK   = 0x40;
const unsigned char Improvements::RAILROAD_MASK   = 0x20;
const unsigned char Improvements::ROAD_MASK       = 0x10;
const unsigned char Improvements::MINING_MASK     = 0x08;
const unsigned char Improvements::IRRIGATION_MASK = 0x04;
const unsigned char Improvements::CITY_MASK       = 0x02;
const unsigned char Improvements::UNIT_MASK       = 0x01;

/////////////////////// WhichCivs Constants ///////////////////////////////
const unsigned char WhichCivs::PURPLE_MASK = 0x80;
const unsigned char WhichCivs::ORANGE_MASK = 0x40;
const unsigned char WhichCivs::CYAN_MASK   = 0x20;
const unsigned char WhichCivs::YELLOW_MASK = 0x10;
const unsigned char WhichCivs::BLUE_MASK   = 0x08;
const unsigned char WhichCivs::GREEN_MASK  = 0x04;
const unsigned char WhichCivs::WHITE_MASK  = 0x02;
const unsigned char WhichCivs::RED_MASK    = 0x01;


/////////////////////// Civ2SavedGame Constants ///////////////////////////////



// MERCATOR
// Added file version constants and offset constants.
// FW_MAP_HEADER_OFFSET renamed to MGE_MAP_HEADER_OFFSET for no good reason
// except to get the two map header offset constants to line up nicely.
const unsigned short CIC_VERSION = 0x27;
const unsigned short FW_VERSION = 0x28;
const unsigned short MGE13_VERSION = 0x2C;
const unsigned short TOT10_VERSION = 0x31;
const unsigned short TOT11_VERSION = 0x32;

const long CIC_MAP_HEADER_OFFSET = 0x3478;
const long MGE_MAP_HEADER_OFFSET = 0x3586;
const long TOT10_TRANSPORTERS_OFFSET = 0x7420;
const long TOT11_TRANSPORTERS_OFFSET = 0x74C8;

/////////////////////// Civ2SavedGame Methods ////////////////////////////////

Civ2SavedGame::Civ2SavedGame()
{
    isMP = false;
    version = 0;
    map_header_offset = 0;
    secondary_maps = 0;
}

Civ2SavedGame::~Civ2SavedGame()
{
    // Destroy maps contained in maps vector
    destroyMaps();
}

// loads a Civ2 Saved game file
//
// Parameters:
// filename        The name of the file to load
//
// Exceptions:
// runtime_error - when the load fails for some reason.  The reason is
//                 returned in the what() member of the runtime_error.

void Civ2SavedGame::load(const string& filename) throw (runtime_error)
{
    ifstream theFile;

    isMP = isMPFile(filename);

    theFile.open(filename.c_str(), ios_base::binary);

    if (!theFile) throw runtime_error(string("Could not open file: ")
                                      +=filename);
    // Seek within a saved game file for the map header
    if (!isMP)
    {
		// MERCATOR
		// Find out offset and seek to it.
		loadMapHeaderOffset(theFile);
		theFile.seekg(map_header_offset);

        if (!theFile) throw runtime_error(string("Error accessing file: ")
                                          +=filename);
    }
    else
    {
        theFile.seekg(0);

        if (!theFile) throw runtime_error(string("Error accessing file: ")
                                          +=filename);
    }

    try
    {
        LogOutput::log() << "Loading File: " << filename << endl;
        loadMapHeader(theFile);
        
        if (isMP)
        {
            // load start positions
            loadStartPositions(theFile);
        }

        // Destroy any previous maps
        destroyMaps();

        // Allocate and load maps. There should always be at least 1
        for (int i = 0; i < secondary_maps+1; i++)
        {
            maps.push_back(new Civ2Map(header->x_dimension,
                                       header->y_dimension,
                                       header->map_area,
                                       !isMP, // The map has a civ view map
                                              // if this is not an MP file
                                       i,
                                       header->flat_earth) );
            maps[i]->load(theFile);

            // Read map specific seed for TOT files
            if (version == TOT10_VERSION || 
                version == TOT11_VERSION )
            {
                 maps[i]->setSeed(loadMapSpecificSeed(theFile));
                 LogOutput::log() << "Map " << i + 1 << " seed is " << maps[i]->getSeed() << endl;
            }
            else
            {
                // Use the global seed
                maps[i]->setSeed(header->map_seed);
            }
        } // end loop over all maps
    }
    catch (runtime_error& e)
    {
        throw runtime_error(string("File: ") + filename + " " + e.what());
    }
}

// Saves to a Civ2 Saved game file
//
// Parameters:
// filename        The name of the file to save over
//
// Exceptions:
// runtime_error - when the save fails for some reason.  The reason is
//                 returned in the what() member of the runtime_error.
//
// Notes: For .SAV (or .SCN) files, this saves over an existing file, and does
//        not create a new file. This is because I currently don't know the full
//        format for a Civ2 Saved game file, and therefore cannot create a
//        complete file from scratch.
//        For MP files, it can create the file, but if the file exists, it will
//        not write over existing civilization start information.

void Civ2SavedGame::save(const string& filename) throw (runtime_error)
{
    fstream theFile;

    // Check for an inconsistent map structure. It is illegal to save with
    // 0 maps, and it is illegal to save with > 1 map if this is not a ToT saved
    // game
    if (maps.size()==0) throw runtime_error("Cannot save a file with no maps");

    if (maps.size() > 1 && supportsMultiMaps() == false)
    {
        throw runtime_error("Cannot save multiple maps into a non-ToT game");
    }

    // The C and C++ standards don't have a way to open a file if it exists
    // (without truncating) or to create the file if it doesn't in a single
    // open call. So I check for its existence first.
    bool exists = fileExists(filename);

    if (isMP)
    {

        if (exists)
        {
           // Open the file as read/write so it will not be truncated if it 
           // exists. From my understanding of the C++ standard this is equivalent
           // to r+b being passed to fopen in C.
           theFile.open(filename.c_str(), ios_base::in | ios_base::out | ios_base::binary);
        }
        else
        {
           // Else open the file for writing only. This will force a new file
           // to be created (same as wb in C)
           theFile.open(filename.c_str(), ios_base::out | ios_base::binary);
        }
    }
    else
    {
        if (!exists)
        {
            // Currently creating a SAV file form scratch is not supported
            throw runtime_error(string("Cannot create SAV/SCN file from scratch: ")
                                += filename);
        }
        else
        {
           // Open the file as read/write to prevent truncating it
           theFile.open(filename.c_str(), ios_base::in | ios_base::out | ios_base::binary);
        }
    }

    if (!theFile) throw runtime_error(string("Could not open file: ")
                                      +=filename);

    if (!isMP)
    {
		loadMapHeaderOffset(theFile);
        theFile.seekp(map_header_offset);

        if (!theFile) throw runtime_error(string("Error accessing file: ")
                                          +=filename);
    }
    else 
    {
        theFile.seekp(0);

        if (!theFile) throw runtime_error(string("Error accessing file: ")
                                          +=filename);
    }

    try
    {
        saveMapHeader(theFile);
        if (isMP)
        {
            saveStartPositions(theFile);
        }

        // Save each map in turn, writing a map specific seed
        // for TOT maps
        for (int i = 0; i < secondary_maps + 1; i++)
        {
            maps[i]->save(theFile);

            if (!isMP && (version == TOT10_VERSION || version == TOT11_VERSION ))
            {
                saveMapSpecificSeed(theFile, maps[i]->getSeed());
            }
        }
    }
    catch (runtime_error& e)
    {
        throw runtime_error(string("File: ") + filename + " " + e.what());
    }

}

// Creates a MP file in memory
void Civ2SavedGame::createMP(int width, int height) throw (runtime_error)
{
    // First, destroy pre-existing maps (if any)
    destroyMaps();

    // Second, allocate memory
    SmartPointer<MapHeader> newHeader = new MapHeader();
    if (newHeader.isNull()) throw runtime_error("Insufficient memory.");

    SmartPointer<StartPositions> newStart = new StartPositions();
    if (newStart.isNull()) throw runtime_error("Insufficient memory.");

    // Third, initialize header and start positions
    newHeader->x_dimension = width;
    newHeader->y_dimension = height;
    newHeader->map_area = (width * height) / 2;
    newHeader->flat_earth = 0;
    newHeader->map_seed = 1;
    newHeader->locator_x_dimension = width/4;
    if (width % 4 != 0) ++newHeader->locator_x_dimension;
    newHeader->locator_y_dimension = height/4;
    if (height % 4 != 0) ++newHeader->locator_y_dimension;

    for (int i = 0; i < 21; i++)
    {
        newStart->x_positions[i] = -1;
        newStart->y_positions[i] = -1;
    }

    // Fourth, allocate a map
    maps.push_back(new Civ2Map(newHeader->x_dimension,
                               newHeader->y_dimension,
                               newHeader->map_area,
                               false, // MPs have no civ view information
                               0, // The first and only map for the .MP file
                               false) ); 

    // Now set members
    isMP = true;

    header = newHeader.releaseControl();
    start_positions = newStart.releaseControl();
    secondary_maps = 0;
    version = 0;
    map_header_offset = 0;
}

// Creates a new saved game file in memory, not supported yet
void Civ2SavedGame::createSAV(int width, int height, int numMaps) throw (runtime_error)
{
    throw runtime_error("Cannot create a new .SAV file.");
}


// Returns the resource seed for the map
unsigned short Civ2SavedGame::getSeed() const throw (runtime_error)
{
    if (header.isNull()) throw runtime_error("No map loaded.");
    return header->map_seed;
}

// Sets the resource seed for the map
void Civ2SavedGame::setSeed(unsigned short seed) throw (runtime_error)
{
    if (header.isNull()) throw runtime_error("No map loaded.");
    header->map_seed = seed;
}

// Returns the map width
int Civ2SavedGame::getWidth() const throw(runtime_error)
{
    return (int)(header->x_dimension);
}
// Returns the map height
int Civ2SavedGame::getHeight() const throw(runtime_error)
{
    return (int)(header->y_dimension);
}

// Return the starting positions for civilizations. This is only
// valid for .MP files
Civ2SavedGame::StartPositions& Civ2SavedGame::getCivStart() throw (runtime_error)
{
    if (!isMP || start_positions.isNull())
    {
        throw runtime_error("Start positions are not defined on a .SAV file.");
    }
    else return *start_positions;
}

// Sets the starting positions for civilizations. This is only valid
// for .MP files
void Civ2SavedGame::setCivStart(const StartPositions& sp) throw (runtime_error)
{
    if (!isMP || start_positions.isNull())
    {
        throw runtime_error("Start positions are not defined on a .SAV file.");
    }
    else
    {
        if (start_positions.isNull())
        {
            start_positions = new StartPositions();
            if (start_positions.isNull())
                throw runtime_error("Insufficient memory.");
        }

        *start_positions = sp;
    }
}


// Static method that returns if a file is a .MP file (has a .MP extension)
bool Civ2SavedGame::isMPFile(string filename)
{
    string copy = copy_to_lower(filename);

    int dot_index = copy.find_last_of('.');
    if (dot_index >= 0 && copy.compare(dot_index, string::npos, ".mp") == 0)
    {
        return true;
    }
    else 
    {
        return false;
    }
}

// Return number of maps in saved game
int Civ2SavedGame::getNumMaps() const throw (runtime_error)
{
    return secondary_maps + 1;
}

// Add a new map to the game. n is an index into the current set of maps
// n must be >= 0 and <= getNumMaps()
void Civ2SavedGame::addMap(int n) throw (runtime_error)
{
    if (supportsMultiMaps() == false)
    {
        throw runtime_error("Cannot add a new map to a file that is not a ToT saved game");
    }
    if (n < 0 || n > secondary_maps + 1)
    {
        stringstream message;
        message << "Cannot add new map, index " << n << " is < 0 or > " 
                 << secondary_maps + 1;
        throw runtime_error(message.str());
    }

    // Create the new map
    SmartPointer<Civ2Map> m = new Civ2Map(header->x_dimension,
                                          header->y_dimension,
                                          header->map_area,
                                          !isMP, 
                                          n,
                                          header->flat_earth); 
    // Insertion at the end is easy for a vector
    if (n == secondary_maps +1)
    {
        maps.push_back(m.releaseControl());
    }
    else // We must iterate through to find the right point
    {
        vector<Civ2Map*>::iterator i = maps.begin();
        for (int j = 0; j <= n; j++)
        {
            if (j == n)
            { 
                // This is inefficient for a vector, but I chose to 
                // give getMap()'s performance a higher priority than
                // addMap()/removeMap()
                maps.insert(i, m.releaseControl());
            }
            else // Must use the else, because insert will invalidate
                 // iterator
            {
                i++;
            }
        }
    }
    secondary_maps++;
}

// Removes map at index n
// n must be >= 0 and < getNumMaps()
void Civ2SavedGame::removeMap(int n) throw (runtime_error)
{
    if (n < 0 || n > secondary_maps)
    {
        stringstream message;
        message << "Cannot remove map, index " << n << " is < 0 or > " 
                 << secondary_maps;
        throw runtime_error(message.str());
    } 
    // delete the map
    Civ2Map *m = maps[n];
    delete m;

    // Now remove that map entry
    if (n == secondary_maps)
    {
        maps.pop_back();
    }
    else // Must iterate through to find the right one to remove
    {
        vector<Civ2Map*>::iterator i = maps.begin();
        for (int j = 0; j <= n; j++)
        {
            if (j == n)
            {
                maps.erase(i);
            }
            else // Must have else because remove invalidates i
            {
                i++;
            }
        }
    }
}

// Return the map at index n
// n must be >= 0 and < getNumMaps()

Civ2Map& Civ2SavedGame::getMap(int n) throw (runtime_error)
{
    if (n < 0 || n > secondary_maps) 
    {
        stringstream message;
        message << "Cannot get map, index " << n << " is < 0 or > " 
                 << secondary_maps;
        throw runtime_error(message.str());
    } 

    return *(maps[n]);
}

////////////////////////// Private Helper Functions ///////////////////////////

// MERCATOR
// Gets the offset of the map header in a savegame.
// Stores values into <version> and <map_header_offset> variables.

void Civ2SavedGame::loadMapHeaderOffset(istream& is) throw (runtime_error)
{
	unsigned int transporters;

	// Go to version number offset (0x0A)
	is.seekg(10);
	if (!is) throw runtime_error("Read error.");

	// Read version into private object variable
	is.read( (char *) &version, sizeof(short));
    if (is.gcount() != sizeof(short)) throw runtime_error("Read error.");

	// Set map header offset depending on file version
	switch(version)
	{
		case CIC_VERSION:
			map_header_offset = CIC_MAP_HEADER_OFFSET;
			break;
		case FW_VERSION:
		case MGE13_VERSION:
			map_header_offset = MGE_MAP_HEADER_OFFSET;
			break;
		case TOT10_VERSION:
			is.seekg(TOT10_TRANSPORTERS_OFFSET);
			if (!is) throw runtime_error("Read error.");

			is.read( (char *) &transporters, sizeof(int));
			if (is.gcount() != sizeof(int)) throw runtime_error("Read error.");

			map_header_offset = TOT10_TRANSPORTERS_OFFSET + 4 + (transporters * 14);
			break;
		case TOT11_VERSION:
			is.seekg(TOT11_TRANSPORTERS_OFFSET);
			if (!is) throw runtime_error("Read error.");

			is.read( (char *) &transporters, sizeof(int));
			if (is.gcount() != sizeof(int)) throw runtime_error("Read error.");

			map_header_offset = TOT11_TRANSPORTERS_OFFSET + 4 + (transporters * 14);
			break;
		default:
        {
			// Whoops... the version needs to be converted to a string somehow.
            stringstream message;
            message << "Unrecognized savegame/scenario version number: " << version;
			throw runtime_error(message.str());
        } // braces needed because message is defined in a switch
	}
}

// Loads the map header from an istream.  This asumes the read pointer for
// the istream is at the correct location.

void Civ2SavedGame::loadMapHeader(istream& is) throw(runtime_error)
{
    SmartPointer<MapHeader> p = new MapHeader;
    if (p == NULL) throw runtime_error("Insufficient Memory.");


    is.read(reinterpret_cast<char *>(p.get()), sizeof(MapHeader));

    if (is.gcount() != sizeof(MapHeader))
        throw runtime_error("Read error.");

    header = p.releaseControl();

	// MERCATOR
	// Also read 8th header value for ToT files.
	if (version == TOT10_VERSION || version == TOT11_VERSION)
	{
		is.read( (char *) &secondary_maps, sizeof(short));

		if (is.gcount() != sizeof(short))
        throw runtime_error("Read error.");
	}

    LogOutput::log() << "X Dimension is: " << header->x_dimension << endl;
    LogOutput::log() << "Y Dimension is: " << header->y_dimension << endl;
    LogOutput::log() << "Map Area is: " << header->map_area << endl;
    LogOutput::log() << "Flat Earth is: " << header->flat_earth << endl;
    LogOutput::log() << "Map Seed is: " << header->map_seed << endl;
    LogOutput::log() << "locator x dim is: " << header->locator_x_dimension << endl;
    LogOutput::log() << "locator y dim is: " << header->locator_y_dimension << endl;

    // MERCATOR
    // Also write 8th header value for ToT files.
    if (version == TOT10_VERSION || version == TOT11_VERSION)
        LogOutput::log() << "Secondary Maps: " << secondary_maps << endl;

}

// Saves a map header to an ostream.

void Civ2SavedGame::saveMapHeader(ostream& os) const throw(runtime_error)
{
    if (header == NULL)
    {
        throw runtime_error("Cannot Save: No map loaded.");
    }

    os.write(reinterpret_cast<const char *>(header.get()), sizeof(MapHeader));
    if (!os)
        throw runtime_error("Write Error.");

    // Write the number of secondary maps for ToT versions
	if (supportsMultiMaps()) 
	{
        os.write(reinterpret_cast<const char *>(&secondary_maps), sizeof(secondary_maps));
        if (!os)
            throw runtime_error("Write Error.");
	}


    LogOutput::log() << "Wrote header." << endl;

}



// Loads the civilization starting positions from a .MP file.
// The istream is assumed to be at the propeer offset.
void Civ2SavedGame::loadStartPositions(istream& is)
                                       throw (runtime_error)
{
    StartPositions *ptr = new StartPositions();
    SmartPointer<StartPositions> p = ptr;                
    if (ptr == NULL) throw runtime_error("Insufficient Memory.");

    is.read(reinterpret_cast<char *>(ptr), sizeof(StartPositions));

    if (is.gcount() != sizeof(StartPositions))
        throw runtime_error("Read error.");

    start_positions = p.releaseControl();
    LogOutput::log() << "Loaded start positions." << endl;
}

// Loads and returns the map specific seed from an input stream
unsigned short Civ2SavedGame::loadMapSpecificSeed(istream& is) throw (runtime_error)
{
    unsigned short seed;

    is.read(reinterpret_cast<char *>(&seed), sizeof(unsigned short));
    
    if (is.gcount() != sizeof(unsigned short))
        throw runtime_error("Read error.");

    return seed;
}

void Civ2SavedGame::saveMapSpecificSeed(ostream& os, unsigned short seed) throw (runtime_error)
{
    os.write(reinterpret_cast<const char *>(&seed), sizeof(unsigned short));
    if (!os)
        throw runtime_error("Write Error.");
}

// Saves a .MP file's starting positions to disk. This assumes os is
// already at the correct offset.
void Civ2SavedGame::saveStartPositions(ostream& os) const throw (runtime_error)
{

    if (start_positions == NULL)
    {
        throw runtime_error("Cannot Save: No map loaded.");
    }

    os.write(reinterpret_cast<const char *>(start_positions.get()), sizeof(StartPositions));
    if (!os)
        throw runtime_error("Write Error.");

    LogOutput::log() << "Wrote starting positions." << endl;
}

// Destroy all memory associated with the maps vector
void Civ2SavedGame::destroyMaps()
{
    for (int i = 0; i < maps.size(); i++)
    {
        delete maps[i];
    }
    maps.clear();
}

// Determine if this saved game file supports multiple maps
bool Civ2SavedGame::supportsMultiMaps() const
{
    return version == TOT10_VERSION || version == TOT11_VERSION;
}   

/////////////////////// Improvements Methods ////////////////////////////////
Improvements::Improvements()
{
    // Initialize to no improvements
    improvements = 0;
}

Improvements::Improvements(const Improvements& i)
{
    improvements = i.improvements;
}

Improvements::Improvements(unsigned char bitfield)
{
    improvements = bitfield;
}

Improvements& Improvements::operator = (const Improvements& i)
{
   improvements = i.improvements;
   return *this;
}
    
bool Improvements::hasUnit()
{
    return (improvements & UNIT_MASK);
}
bool Improvements::hasCity()
{
    return (improvements & CITY_MASK);
}
bool Improvements::hasIrrigation()
{
    return (improvements & IRRIGATION_MASK);
}
bool Improvements::hasMining()
{
    return (improvements & MINING_MASK);
}
bool Improvements::hasRoad()
{
    return (improvements & ROAD_MASK);
}
bool Improvements::hasRailroad()
{
    return (improvements & RAILROAD_MASK);
}
bool Improvements::hasFortress()
{
    return (improvements & FORTRESS_MASK);
}
bool Improvements::hasPollution()
{
    return (improvements & POLLUTION_MASK);
}
   
void Improvements::setUnit(bool b)
{
    if (b) improvements |= UNIT_MASK;
    else improvements &= (~UNIT_MASK);
}

void Improvements::setCity(bool b)
{
    if (b) improvements |= CITY_MASK;
    else improvements &= (~CITY_MASK);
}
void Improvements::setIrrigation(bool b)
{
    if (b) improvements |= IRRIGATION_MASK;
    else improvements &= (~IRRIGATION_MASK);
}
void Improvements::setMining(bool b)
{
    if (b) improvements |= MINING_MASK;
    else improvements &= (~MINING_MASK);
}
void Improvements::setRoad(bool b)
{
    if (b) improvements |= ROAD_MASK;
    else improvements &= (~ROAD_MASK);
}
void Improvements::setRailroad(bool b)
{
    if (b) improvements |= RAILROAD_MASK;
    else improvements &= (~RAILROAD_MASK);
}
void Improvements::setFortress(bool b)
{
    if (b) improvements |= FORTRESS_MASK;
    else improvements &= (~FORTRESS_MASK);
}
void Improvements::setPollution(bool b)
{
    if (b) improvements |= POLLUTION_MASK;
    else improvements &= (~POLLUTION_MASK);
}

/////////////////////// WhichCivs Methods ////////////////////////////////
WhichCivs::WhichCivs()
{
    // Initialize to no WhichCivs
    whichCivs = 0;
}

WhichCivs::WhichCivs(const WhichCivs& i)
{
    whichCivs = i.whichCivs;
}

WhichCivs::WhichCivs(unsigned char bitfield)
{
    whichCivs = bitfield;
}

WhichCivs& WhichCivs::operator = (const WhichCivs& i)
{
   whichCivs = i.whichCivs;
   return *this;
}
    
bool WhichCivs::hasRed()
{
    return (whichCivs & RED_MASK);
}
bool WhichCivs::hasWhite()
{
    return (whichCivs & WHITE_MASK);
}
bool WhichCivs::hasGreen()
{
    return (whichCivs & GREEN_MASK);
}
bool WhichCivs::hasBlue()
{
    return (whichCivs & BLUE_MASK);
}
bool WhichCivs::hasYellow()
{
    return (whichCivs & YELLOW_MASK);
}
bool WhichCivs::hasCyan()
{
    return (whichCivs & CYAN_MASK);
}
bool WhichCivs::hasOrange()
{
    return (whichCivs & ORANGE_MASK);
}
bool WhichCivs::hasPurple()
{
    return (whichCivs & PURPLE_MASK);
}
   
void WhichCivs::setRed(bool b)
{
    if (b) whichCivs |= RED_MASK;
    else whichCivs &= (~RED_MASK);
}

void WhichCivs::setWhite(bool b)
{
    if (b) whichCivs |= WHITE_MASK;
    else whichCivs &= (~WHITE_MASK);
}
void WhichCivs::setGreen(bool b)
{
    if (b) whichCivs |= GREEN_MASK;
    else whichCivs &= (~GREEN_MASK);
}
void WhichCivs::setBlue(bool b)
{
    if (b) whichCivs |= BLUE_MASK;
    else whichCivs &= (~BLUE_MASK);
}
void WhichCivs::setYellow(bool b)
{
    if (b) whichCivs |= YELLOW_MASK;
    else whichCivs &= (~YELLOW_MASK);
}
void WhichCivs::setCyan(bool b)
{
    if (b) whichCivs |= CYAN_MASK;
    else whichCivs &= (~CYAN_MASK);
}
void WhichCivs::setOrange(bool b)
{
    if (b) whichCivs |= ORANGE_MASK;
    else whichCivs &= (~ORANGE_MASK);
}
void WhichCivs::setPurple(bool b)
{
    if (b) whichCivs |= PURPLE_MASK;
    else whichCivs &= (~PURPLE_MASK);
}
