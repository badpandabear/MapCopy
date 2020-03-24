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

#include "civ2sav.h"

/////////////////////// Civ2SavedGame Constants ///////////////////////////////

const long FW_MAP_HEADER_OFFSET = 0x3586;
const unsigned char RIVER_FLAG = 0x80;
const unsigned char NO_RESOURCE_FLAG = 0x40;
const unsigned char TERRAIN_TYPE_MASK = 0x3F;

// These are the weights we use when calculating fertility values
// These are based on my observations of how the AI does things,
// in an attempt to calculate fertility in as close a manner to the AI's
// algorithm as possible.
const int FOOD_WEIGHT = 3;
const int TRADE_WEIGHT = 2;
const int SHIELD_WEIGHT = 1;

/////////////////////// Civ2SavedGame Methods ////////////////////////////////

Civ2SavedGame::Civ2SavedGame()
{
    terrain_map=NULL;
    civ_view_map=NULL;
    verbose = false;
    isMP = false;
}

Civ2SavedGame::~Civ2SavedGame()
{
    // Smart pointers do all the work
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

    theFile.open(filename.c_str(), ios_base::binary | ios_base::nocreate);

    if (!theFile) throw runtime_error(string("Could not open file: ")
                                      +=filename);
    // Seek within a saved game file for the map header
    if (!isMP)
    {
        theFile.seekg(FW_MAP_HEADER_OFFSET);

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
        if (verbose) cout << "Loading File: " << filename << endl;
        loadMapHeader(theFile);

        if (isMP)
        {
            // load start positions
            loadStartPositions(theFile);
        }
        else
        {
            // Read civ_view map
            loadCivViewMap(theFile);
        }

        loadTerrainMap(theFile);
    }
    catch (runtime_error& e)
    {
        throw runtime_error(string("File: ") + filename + e.what());
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
    ofstream theFile;

    if (isMP)
    {
        theFile.open(filename.c_str(), ios_base::binary);
    }
    else
    {
        theFile.open(filename.c_str(), ios_base::binary | ios_base::nocreate | ios::ate);
    }

    if (!theFile) throw runtime_error(string("Could not open file: ")
                                      +=filename);

    if (!isMP)
    {
        theFile.seekp(FW_MAP_HEADER_OFFSET);

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
        else
        {
            saveCivViewMap(theFile);
        }
        saveTerrainMap(theFile);
    }
    catch (runtime_error& e)
    {
        throw runtime_error(string("File: ") + filename + e.what());
    }

}

// Creates a MP file in memory
void Civ2SavedGame::createMP(int width, int height) throw (runtime_error)
{
    // First, allocate memory
    SmartPointer<MapHeader> newHeader = new MapHeader();
    if (newHeader.isNull()) throw runtime_error("Insufficient memory.");

    SmartPointer<StartPositions> newStart = new StartPositions();
    if (newStart.isNull()) throw runtime_error("Insufficient memory.");

    SmartPointer<TerrainCell, true> newTerrain = new TerrainCell[width * height];
    if (newTerrain.isNull()) throw runtime_error("Insufficient memory.");

    // second initialize
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

    TerrainCell blank;
    blank.terrainType = OCEAN;
    blank.improvements.unit_present = 0;
    blank.improvements.city_present = 0;
    blank.improvements.irrigation = 0;
    blank.improvements.mining = 0;
    blank.improvements.road = 0;
    blank.improvements.railroad = 0;
    blank.improvements.fortress = 0;
    blank.improvements.pollution = 0;
    blank.city_radius = 0;
    blank.body_counter = 0;
    blank.visibility.red = 0;
    blank.visibility.white = 0;
    blank.visibility.green = 0;
    blank.visibility.blue = 0;
    blank.visibility.yellow = 0;
    blank.visibility.cyan = 0;
    blank.visibility.orange = 0;
    blank.visibility.purple = 0;
    blank.fertility = 0;
    blank.ownership = 0xF;

    for (int i = 0; i < newHeader->map_area; i++)
    {
        newTerrain[i] = blank;
    }

    // Now set members
    isMP = true;

    header = newHeader.releaseControl();
    start_positions = newStart.releaseControl();
    terrain_map = newTerrain.releaseControl();

}
// Creates a new saved game file in memory, not supported yet
void Civ2SavedGame::createSAV(int width, int height) throw (runtime_error)
{
    throw runtime_error("Cannot create a new .SAV file.");
}

// Returns the map width
int Civ2SavedGame::getWidth() const throw(runtime_error)
{
    if (header.isNull())
    {
        throw runtime_error("No map loaded");
    }
    return (int)(header->x_dimension);
}
// Returns the map height
int Civ2SavedGame::getHeight() const throw(runtime_error)
{
    if (header.isNull())
    {
        throw runtime_error("No map loaded");
    }
    return (int)(header->y_dimension);
}

// Returns whether a given map cell contains a river
bool Civ2SavedGame::isRiver(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    TerrainCell& c = terrain_map[offset];

    return (c.terrainType & RIVER_FLAG) != 0;
}

// Sets whether a given map cell contains a river
void Civ2SavedGame::setRiver(int x, int y, bool river) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    TerrainCell& c = terrain_map[offset];

    if (river)
    {
        c.terrainType |= RIVER_FLAG;
    }
    else
    {
        c.terrainType &= (~RIVER_FLAG);
    }
}

// Returns whether a resource is hidden at a given map square.
// If this value is true, AND there would normally be a resource at the given
// square, then that resource is hidden.
bool Civ2SavedGame::isResourceHidden(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    TerrainCell& c = terrain_map[offset];

    return (c.terrainType & NO_RESOURCE_FLAG) != 0;
}

// Set whether a resource is hidden at a given map square.
// If this value is set to true, AND there would normally be a resource at the
// given square, then that resource is hidden.  It cannot be used to create
// new resources that don't fit into the pattern defined by the resource seed.

void Civ2SavedGame::setResourceHidden(int x, int y, bool hidden) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    TerrainCell& c = terrain_map[offset];

    if (hidden)
    {
        c.terrainType |= NO_RESOURCE_FLAG;
    }
    else
    {
        c.terrainType &= (~NO_RESOURCE_FLAG);
    }
}

// Returns the terrain type (e.g. mountain, ocean, etc) index for a given map
// square. It does not contain information about rivers or resources or
// improvements

int Civ2SavedGame::getTypeIndex(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x,y);

    TerrainCell& c = terrain_map[offset];

    return (int)(c.terrainType & TERRAIN_TYPE_MASK);
}

// Sets the terrain type (e.g. mountain, ocean, etc) index for a given map
// square.
void Civ2SavedGame::setTypeIndex(int x, int y, int i) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x,y);

    TerrainCell& c = terrain_map[offset];

    unsigned char index = (unsigned char)i;

    c.terrainType = (c.terrainType & (~TERRAIN_TYPE_MASK))
                    | (index & TERRAIN_TYPE_MASK);
}

// Sets the "verbose" flag for the class, which controls whether
// extra information is printed to standard output
void Civ2SavedGame::setVerbose(bool verbose)
{
    this->verbose = verbose;
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

// Returns the improvments on a given terrain square

Civ2SavedGame::Improvements Civ2SavedGame::getImprovements(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return terrain_map[offset].improvements;
}

// Sets the improvments on a given terrain square.
void Civ2SavedGame::setImprovements(int x, int y, Improvements i) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    terrain_map[offset].improvements = i;
}

// return which civs have explored a given square
Civ2SavedGame::WhichCivs Civ2SavedGame::getVisibility(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return terrain_map[offset].visibility;
}
// Set which civs have explored a given square
void Civ2SavedGame::setVisibility(int x, int y, WhichCivs c) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    terrain_map[offset].visibility = c;
}

// Return the fertility of a given square. Fertility ranges from 0 to 16,
// and represents the desireability of a square for building a city
unsigned char Civ2SavedGame::getFertility(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return terrain_map[offset].fertility;
}

// Sets the fertility for a given square
void Civ2SavedGame::setFertility(int x, int y, unsigned char f) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    terrain_map[offset].fertility = f;
}

// Calculates the fertility of a given square based on the surrounding
// terrain
void Civ2SavedGame::calcFertility(int x, int y) throw (runtime_error)
{
    if (terrain_map.isNull()) throw runtime_error("No map loaded.");

    int offset = XYtoOffset(x, y);

    // First find the total max food production of each
    // square in the city radius. This does not consider special resources,
    // but does consider all possible terrain improvements
    // First find this sum, and also find if this square is within a city's
    // radius.
    int resourceSum = 0;
    bool inCityRadius = false;
    vector<Tuple> citySquares;
    getCityRadius(x, y, citySquares);

    int terrainResources[OCEAN+1][3] =
    {
        //              Food Shields Trade
        /* Dessert */   { 2,   1,      1 },
        /* Plains  */   { 3,   1,      1 },
        /* Grassland */ { 4,   0,      1 },
        /* Forest */    { 1,   3,      0 },
        /* Hills   */   { 2,   4,      1 },
        /* Mountains */ { 0,   3,      0 },
        /* Tundra    */ { 1,   0,      0 },
        /* Glacier */   { 0,   1,      0 },
        /* Swamp   */   { 1,   0,      0 },
        /* Jungle  */   { 1,   0,      0 },
        /* Ocean */     { 1,   0,      2 }
    };
    for (unsigned int i = 0; i < citySquares.size(); i++)
    {
        int x = citySquares[i].x;
        int y = citySquares[i].y;
        int type = getTypeIndex(x, y);
        // calculate weighted sum
        resourceSum    += FOOD_WEIGHT * terrainResources[type][0];
        resourceSum    += SHIELD_WEIGHT * terrainResources[type][1];
        resourceSum    += TRADE_WEIGHT * terrainResources[type][2];
        // Rivers add 1 trade
        if (isRiver(x, y))
        {
            resourceSum += TRADE_WEIGHT * 1;
        }
        if (getImprovements(x, y).city_present == 1) inCityRadius = true;
    }

    // The fertility is calculated as a value between 0 and 15 (0x0F).
    // This is calculated a normalized value from 0 to 1 times 15.
    // The normalized value is calculated by dividing by 
    // (42 * FOOD_WEIGHT + 42 * SHIELD_WEIGHT + 42 * TRADE_WEIGHT)
    // 42 was chosen as it represents a production of two per square in the city
    // radius.  Therefore, a square that produces 2 food per square, 2 shields
    // per square, and 2 trade per square gets a full score. (Other combinations
    // of food/trade/shields can also produce a full score).
    double fertility = resourceSum;
    fertility *= 15;
    fertility /= 252;

    // This result may be > than 15, so force it to be 15 and convert to an
    // integer
    if (fertility > 15) fertility = 15;
    unsigned short int f = fertility;

    int type = getTypeIndex(x, y);

    // If we are within a city radius 7 should be the maximum.
    if (inCityRadius) f &= 0x07;

    // Otherwise make sure that a grassland square outside of a city has at least 
    // an 8 fertility. This is to maintain at least some consitency with how
    // Civ 2 calculates fertility
    else if (type == GRASSLAND || type == PLAINS)
    {
        if (f < 8) f = 8;
    }
    else f &= 0x0F;

    terrain_map[offset].fertility = f;
}

// Adjusts the fertility of a given square so that it is in the range
// of 0-7 if it is near a city.
void Civ2SavedGame::adjustFertility(int x, int y) throw (runtime_error)
{
    if (terrain_map.isNull()) throw runtime_error("No map loaded.");

    int offset = XYtoOffset(x, y);

    vector<Tuple> citySquares;
    getCityRadius(x, y, citySquares);

    bool inCityRadius = false;

    for (unsigned int i = 0; i < citySquares.size(); i++)
    {
        int x = citySquares[i].x;
        int y = citySquares[i].y;
        if (getImprovements(x, y).city_present == 1) inCityRadius = true;
    }
    unsigned int f = terrain_map[offset].fertility;

    if (inCityRadius) f &= 0x07;
    else f &= 0x0F;

    terrain_map[offset].fertility = f;
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

// Gets the ownership of a square. This is set for the civilization that
// has a unit/city on or close to a square.
Civ2SavedGame::Civilization Civ2SavedGame::getOwnership(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return terrain_map[offset].ownership;
}

// Sets the ownership of a given square
void Civ2SavedGame::setOwnership(int x, int y, Civilization civ) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    terrain_map[offset].ownership = civ;
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

unsigned char Civ2SavedGame::getBodyCounter(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return terrain_map[offset].body_counter;
}

void Civ2SavedGame::setBodyCounter(int x, int y, unsigned char bc) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    terrain_map[offset].body_counter = bc;
}

Civ2SavedGame::Civilization Civ2SavedGame::getCityRadius(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    // The city radius is stored as the civ # shifted left by 5.
    return (terrain_map[offset].city_radius >> 5);
}

void Civ2SavedGame::setCityRadius(int x, int y, Civilization c) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }
    int offset = XYtoOffset(x, y);

    terrain_map[offset].city_radius = (c << 5);
}


Civ2SavedGame::Improvements Civ2SavedGame::getCivView(int x, int y, Civilization c) const throw (runtime_error)
{
    if (civ_view_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoCivViewOffset(x, y, c);

    return civ_view_map[offset];
}

void Civ2SavedGame::setCivView(int x, int y, Civilization c, Improvements i) throw (runtime_error)
{
    if (civ_view_map.isNull())
    {
        throw runtime_error("No map loaded");
    }
    if (c == ALL)
    {
        setCivView(x, y, WHITE, i);
        setCivView(x, y, GREEN, i);
        setCivView(x, y, BLUE, i);
        setCivView(x, y, YELLOW, i);
        setCivView(x, y, CYAN, i);
        setCivView(x, y, ORANGE, i);
        setCivView(x, y, PURPLE, i);
    }
    else
    {
        int offset = XYtoCivViewOffset(x, y, c);
        civ_view_map[offset] = i;
    }
}

////////////////////////// Private Helper Functions ///////////////////////////

// Loads the map header from an istream.  This asumes the read pointer for
// the istream is at the correct location.

void Civ2SavedGame::loadMapHeader(istream& is) throw(runtime_error)
{
    SmartPointer<MapHeader> p = new MapHeader;
    if (p == NULL) throw runtime_error("Insufficient Memory.");


    is.read(reinterpret_cast<unsigned char *>(p.get()), sizeof(MapHeader));

    if (is.gcount() != sizeof(MapHeader))
        throw runtime_error("Read error.");

    header = p.releaseControl();

    if (verbose)
    {
        cout << "X Dimension is: " << header->x_dimension << endl;
        cout << "Y Dimension is: " << header->y_dimension << endl;
        cout << "Map Area is: " << header->map_area << endl;
        cout << "Flat Earth is: " << header->flat_earth << endl;
        cout << "Map Seed is: " << header->map_seed << endl;
        cout << "locator x dim is: " << header->locator_x_dimension << endl;
        cout << "locator y dim is: " << header->locator_y_dimension << endl;
    }
}

// This loads the terrain map from the given input stream. It assumes that the
// read pointer for the input stream is set to the correct location.
// The read map is placed into terran_map
// Note: What I call a "terrain map" is the second block of map data within the
// Civ2 Saved Game file.  It is also stored in .MP files.

void Civ2SavedGame::loadTerrainMap(istream& is) throw (runtime_error)
{
    if (header == NULL) throw runtime_error("Could not Access Map Header.");

    SmartPointer<TerrainCell, true> p = new TerrainCell[header->map_area];
    if (p == NULL) throw runtime_error("Insufficient Memory.");

    is.read(reinterpret_cast<unsigned char *>(p.get()),
            header->map_area * sizeof(TerrainCell));
    if (is.gcount() != header->map_area * sizeof(TerrainCell))
        throw runtime_error("Read Error.");

    terrain_map = p.releaseControl();
    if (verbose) cout << "Read terrain map\n";
}

// Saves a map header to an ostream.

void Civ2SavedGame::saveMapHeader(ostream& os) const throw(runtime_error)
{
    if (header == NULL)
    {
        throw runtime_error("Cannot Save: No map loaded.");
    }

    os.write(reinterpret_cast<const unsigned char *>(header.get()), sizeof(MapHeader));
    if (!os)
        throw runtime_error("Write Error.");

    if (verbose) cout << "Wrote header "  << endl;

}


// Saves a terrain map to an ostream
void Civ2SavedGame::saveTerrainMap(ostream& os) const throw(runtime_error)
{
    if (header == NULL || terrain_map == NULL)
    {
        throw runtime_error("Cannot Save: No map loaded.");
    }

    os.write(reinterpret_cast<const unsigned char *>(terrain_map.get()),
            header->map_area * sizeof(TerrainCell));
    if (!os)
        throw runtime_error("Write Error.");

    if (verbose) cout << "Wrote terrain map "  << endl;
}

// Loads the civilization starting positions from a .MP file.
// The istream is assumed to be at the propeer offset.
void Civ2SavedGame::loadStartPositions(istream& is)
                                       throw (runtime_error)
{
    StartPositions *ptr = new StartPositions();
    SmartPointer<StartPositions> p = ptr;                
    if (ptr == NULL) throw runtime_error("Insufficient Memory.");

    is.read(reinterpret_cast<unsigned char *>(ptr), sizeof(StartPositions));

    if (is.gcount() != sizeof(StartPositions))
        throw runtime_error("Read error.");

    start_positions = p.releaseControl();
    if (verbose)
    {
        cout << "Loaded start positions." << endl;
    }
}

// Saves a .MP file's starting positions to disk. This assumes os is
// already at the correct offset.
void Civ2SavedGame::saveStartPositions(ostream& os) const throw (runtime_error)
{

    if (start_positions == NULL)
    {
        throw runtime_error("Cannot Save: No map loaded.");
    }

    os.write(reinterpret_cast<const unsigned char *>(start_positions.get()), sizeof(StartPositions));
    if (!os)
        throw runtime_error("Write Error.");

    if (verbose) cout << "Wrote starting positions.\n";
}

// Loads the Civ View map from a .SAV file. Assumes is is already at the correct
// offset.
void Civ2SavedGame::loadCivViewMap(istream& is)
                                   throw (runtime_error)
{
    if (header == NULL) throw runtime_error("Could not Access Map Header.");

    Improvements *ptr = new Improvements[header->map_area * 7];
    SmartPointer<Improvements, true> p = ptr;

    if (ptr == NULL) throw runtime_error("Insufficient Memory.");

    is.read(reinterpret_cast<unsigned char *>(ptr),
            header->map_area * sizeof(Improvements) * 7);
    if (is.gcount() != header->map_area * sizeof(Improvements) * 7)
        throw runtime_error("Read Error.");

    civ_view_map = p.releaseControl();

    if (verbose) cout << "Read civ view map.\n";
}

// Saves the CivView to a .SAV file. Assumes os is already at the correct offset.
void Civ2SavedGame::saveCivViewMap(ostream& os) const
                                   throw (runtime_error)
{
    if (header == NULL || civ_view_map == NULL)
    {
        throw runtime_error("Cannot Save: No map loaded.");
    }

    os.write(reinterpret_cast<const unsigned char *>(civ_view_map.get()),
            header->map_area * sizeof(Improvements) * 7);
    if (!os)
        throw runtime_error("Write Error.");

    if (verbose) cout << "Wrote civ_view_map " << endl;
    
}


// Converts an X,Y coordinate to an offset within the terrain map. As in
// Civ2, Some x and y values are not valid (in particular, x + y must be
// even).
// If a conversion from offset to x,y coordinates is ever needed,
// it would work like this:
/*
    width = header->x_dimension/2;
    y = i /  width
    x = (i % width) * 2          if y is even
        ((i % width) * 2) + 1    if y is odd
*/

int Civ2SavedGame::XYtoOffset(int x, int y) const
                              throw (runtime_error)
{
    if (header == NULL)
    {
        throw runtime_error("No map loaded");
    }
    if ((x + y) % 2 != 0)
        throw runtime_error("Invalid x,y coordinates, x+y must be even.");

    int offset = x/2 + (y * (header->x_dimension/2));

    if (offset <0 || offset >= header->map_area)
        throw runtime_error("Invalid x,y coordinates: out of bounds.");

    return offset;
}

// Converts an X,Y coordinate to an offset within the civ_view_map. As in
// Civ2, Some x and y values are not valid (in particular, x + y must be
// even).

int Civ2SavedGame::XYtoCivViewOffset(int x, int y, Civilization c) const
                                     throw (runtime_error)
{
    // Do not accept barbarians, there is now CivView information stored for
    // them
    if (c == RED) throw runtime_error("Barbarians do not have civ view info.");

    // use the terrain_map offset, and adjust for civilization
    int offset = XYtoOffset(x, y);

    // C++ won't let us subtract down the value of an enumeration
    int civ = c;

    offset += (civ-1) * header->map_area;

    return offset;
}

// Adds Tuples to vector "out" for each square in the city radius around x, y
void Civ2SavedGame::getCityRadius(int x, int y, vector<Tuple>& out) const
                                  throw (runtime_error)
{
    // Offsets for calculating the 21 squares in a city radius.
    Tuple offsets[21] = { Tuple(-3, -1), Tuple(-2, -2), Tuple(-1, -3),
                          Tuple(-3,  1), Tuple(-2,  0), Tuple(-1, -1),
                          Tuple( 0, -2), Tuple( 1, -3), Tuple(-2,  2),
                          Tuple(-1, -1), Tuple( 0,  0), Tuple( 1, -1),
                          Tuple( 2, -2), Tuple(-1,  3), Tuple( 0,  2),
                          Tuple( 1,  1), Tuple( 2,  0), Tuple( 3, -1),
                          Tuple( 1,  3), Tuple( 2,  2), Tuple( 3,  1) };

    // Iterate through each square in the city radius
    for (int i = 0; i < 21; i++)
    {
        // Add the offsets to find the x,y coordinates of the square
        Tuple temp(x + offsets[i].x, y + offsets[i].y);

        // Check for out of bounds
        if (temp.y < 0 || temp.y >= header->y_dimension) continue;

        // When checking for x being out of bounds, do wrapping if the world
        // is round
        if (temp.x < 0)
        {
            if (header->flat_earth == 0)
            {
                temp.x = header->x_dimension - (temp.x % header->x_dimension);
            }
            else continue;
        }
        if (temp.x >= header->x_dimension)
        {
            if (header->flat_earth == 0)
            {
                temp.x %= header->x_dimension;
            }
            else continue;
        }

        // Insert the resulting coordinates in the vector
        out.push_back(temp);
    }
}

