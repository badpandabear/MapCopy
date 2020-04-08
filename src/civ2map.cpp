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

// civ2map.cpp
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
// Feb/13/2005 JDR  Split off Civ2Map.cpp from Civ2Sav.cpp
// Feb/20/2005 JDR  1.2Beta1 release of ToT multi-map support

#include <iostream>

#include "civ2sav.h"

/////////////////////// Civ2Map Constants ///////////////////////////////


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

/////////////////////// Civ2Map Constructor ////////////////////////////////

// Private constructor called only by Civ2SavedGame
Civ2Map::Civ2Map(int x_dim, int y_dim, int area, bool has_civ_view_map,
                 int ma_pos, bool fe) throw (runtime_error)
{
    x_dimension = x_dim;
    y_dimension = y_dim;
    map_area = area;
    has_civ_view = has_civ_view_map;
    map_seed = 1; // 1 causes Civ2 to randomly generate a new seed
    flat_earth = fe;

    // I made map_position an unsigned char to avoid interger conversions
    // when doing the body counter adjustments
    map_position = static_cast<unsigned char> (ma_pos);

    // Allocate terrain map
    terrain_map = new TerrainCell[x_dim * y_dim];
    if (terrain_map.isNull()) throw runtime_error("Insufficient memory.");

    // Allocate civ view map if needed
    if (has_civ_view_map)
    {
        civ_view_map = new unsigned char[map_area * 7];
        if (civ_view_map.isNull()) throw runtime_error("Insufficient memory.");

        // Initialize
        for (int i = 0; i  < map_area * 7; i++)
        {
            civ_view_map[i] = 0;
        }
    }
}

/////////////////////// Civ2Map Public methods ////////////////////////////////

// Load terrain and civ view specific information from an input stream
void Civ2Map::load(istream& is) throw (runtime_error)
{
    // The Civ specific view map comes first, if it exists
    if (has_civ_view)
    {
        loadCivViewMap(is);
    }

    loadTerrainMap(is);
}

// Save terrain and civ view specific information to an output stream
void Civ2Map::save(ostream& os) throw (runtime_error)
{
    // The Civ specific view map comes first, if it exists
    if (has_civ_view)
    {
        saveCivViewMap(os);
    }

    saveTerrainMap(os);
}

// Returns the map width
int Civ2Map::getWidth() const throw(runtime_error)
{
    return (int)(x_dimension);
}
// Returns the map height
int Civ2Map::getHeight() const throw(runtime_error)
{
    return (int)(y_dimension);
}

// Returns whether a given map cell contains a river
bool Civ2Map::isRiver(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    const TerrainCell& c = terrain_map[offset];

    return (c.terrainType & RIVER_FLAG) != 0;
}

// Sets whether a given map cell contains a river
void Civ2Map::setRiver(int x, int y, bool river) throw (runtime_error)
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
bool Civ2Map::isResourceHidden(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    const TerrainCell& c = terrain_map[offset];

    return (c.terrainType & NO_RESOURCE_FLAG) != 0;
}

// Set whether a resource is hidden at a given map square.
// If this value is set to true, AND there would normally be a resource at the
// given square, then that resource is hidden.  It cannot be used to create
// new resources that don't fit into the pattern defined by the resource seed.

void Civ2Map::setResourceHidden(int x, int y, bool hidden) throw (runtime_error)
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

int Civ2Map::getTypeIndex(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x,y);

    const TerrainCell& c = terrain_map[offset];

    return (int)(c.terrainType & TERRAIN_TYPE_MASK);
}

// Sets the terrain type (e.g. mountain, ocean, etc) index for a given map
// square.
void Civ2Map::setTypeIndex(int x, int y, int i) throw (runtime_error)
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

// Returns the resource seed for the map
unsigned short Civ2Map::getSeed() const throw (runtime_error)
{
    return map_seed;
}

// Sets the resource seed for the map
void Civ2Map::setSeed(unsigned short seed) throw (runtime_error)
{
    map_seed = seed;
}

// Returns the improvments on a given terrain square

Improvements Civ2Map::getImprovements(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return Improvements(terrain_map[offset].improvements);
}

// Sets the improvments on a given terrain square.
void Civ2Map::setImprovements(int x, int y, Improvements i) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    terrain_map[offset].improvements = i.improvements;
}

// return which civs have explored a given square
WhichCivs Civ2Map::getVisibility(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return WhichCivs(terrain_map[offset].visibility);
}
// Set which civs have explored a given square
void Civ2Map::setVisibility(int x, int y, WhichCivs c) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    terrain_map[offset].visibility = c.whichCivs;
}

// Return the fertility of a given square. Fertility ranges from 0 to 16,
// and represents the desireability of a square for building a city
unsigned char Civ2Map::getFertility(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return (terrain_map[offset].fert_ownership & 0x0F);
}

// Sets the fertility for a given square
void Civ2Map::setFertility(int x, int y, unsigned char f) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);
    terrain_map[offset].fert_ownership = 
        (terrain_map[offset].fert_ownership & 0xF0) | (f & 0x0F);
}

// Calculates the fertility of a given square based on the surrounding
// terrain
void Civ2Map::calcFertility(int x, int y) throw (runtime_error)
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
        if (getImprovements(x, y).hasCity()) inCityRadius = true;
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
    unsigned short int f = static_cast<unsigned short int>(fertility);

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

    terrain_map[offset].fert_ownership = 
        (terrain_map[offset].fert_ownership & 0xF0) | f;
}

// Adjusts the fertility of a given square so that it is in the range
// of 0-7 if it is near a city.
void Civ2Map::adjustFertility(int x, int y) throw (runtime_error)
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
        if (getImprovements(x, y).hasCity()) inCityRadius = true;
    }
    unsigned int f = terrain_map[offset].fert_ownership & 0x0F;

    if (inCityRadius) f &= 0x07;
    else f &= 0x0F;

    terrain_map[offset].fert_ownership = 
        (terrain_map[offset].fert_ownership & 0xF0) | f;
}

// Gets the ownership of a square. This is set for the civilization that
// has a unit/city on or close to a square.
Civ2Map::Civilization Civ2Map::getOwnership(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return static_cast<Civilization>(terrain_map[offset].fert_ownership >> 4);
}

// Sets the ownership of a given square
void Civ2Map::setOwnership(int x, int y, Civilization civ) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);
    unsigned char c = static_cast<unsigned char>(civ);
    terrain_map[offset].fert_ownership =  
        (terrain_map[offset].fert_ownership & 0x0F) | (c << 4);
}

unsigned char Civ2Map::getBodyCounter(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    return terrain_map[offset].body_counter;
}

void Civ2Map::setBodyCounter(int x, int y, unsigned char bc) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    // The body counter must be adjusted to the map postion in the map vector
    // for Map 0,   0- 63 is allowed
    // for Map 1   64-127 is allowed
    // for Map 2  128-191 is allowed
    // for Map 3  192-255 is allowed
    // Perhaps by design, this means the upper two bits of the body_counter
    // represent the map index.  I'll rely on this fact to adjust the 
    // body_counter being passed in
    bc = bc & 0x3F; // Remove the two highest bits
    bc = bc | (map_position << 6); // Set the highest two bits based on map position

    terrain_map[offset].body_counter = bc;
}

Civ2Map::Civilization Civ2Map::getCityRadius(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x, y);

    // The city radius is stored as the civ # shifted left by 5.
    return static_cast<Civilization>(terrain_map[offset].city_radius >> 5);
}

void Civ2Map::setCityRadius(int x, int y, Civilization c) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }
    int offset = XYtoOffset(x, y);

    terrain_map[offset].city_radius = (c << 5);
}


Improvements Civ2Map::getCivView(int x, int y, Civilization c) const throw (runtime_error)
{
    if (civ_view_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoCivViewOffset(x, y, c);

    return Improvements(civ_view_map[offset]);
}

void Civ2Map::setCivView(int x, int y, Civilization c, Improvements i) throw (runtime_error)
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
        civ_view_map[offset] = i.improvements;
    }
}

////////////////////////// Private Methods ///////////////////////////

// This loads the terrain map from the given input stream. It assumes that the
// read pointer for the input stream is set to the correct location.
// The read map is placed into terran_map
// Note: What I call a "terrain map" is the second block of map data within the
// Civ2 Saved Game file.  It is also stored in .MP files.

void Civ2Map::loadTerrainMap(istream& is) throw (runtime_error)
{
    if (terrain_map == NULL)
    {
        throw runtime_error("Cannot Save: No map allocated.");
    }

    is.read(reinterpret_cast<char *>(terrain_map.get()),
            map_area * sizeof(TerrainCell));
    if (is.gcount() != map_area * sizeof(TerrainCell))
        throw runtime_error("Read Error.");

    LogOutput::log() << "Read terrain map" << endl;
}

// Saves a terrain map to an ostream
void Civ2Map::saveTerrainMap(ostream& os) const throw(runtime_error)
{
    if (terrain_map == NULL)
    {
        throw runtime_error("Cannot Save: No map allocated.");
    }

    os.write(reinterpret_cast<const char *>(terrain_map.get()),
            map_area * sizeof(TerrainCell));

    if (!os)
        throw runtime_error("Write Error.");

    LogOutput::log() << "Wrote terrain map." << endl;
}

// Loads the Civ View map from a .SAV file. Assumes is is already at the correct
// offset.
void Civ2Map::loadCivViewMap(istream& is)
                                   throw (runtime_error)
{
    if (has_civ_view == false)
    {
        throw runtime_error("Cannot load civ view map because it is not allowed.");
    }

    if (civ_view_map.isNull()) throw runtime_error("Cannot load civ view map because it is not allocated.");

    is.read(reinterpret_cast<char *>(civ_view_map.get()),
            map_area * sizeof(unsigned char) * 7);
    if (is.gcount() != map_area * sizeof(unsigned char) * 7)
        throw runtime_error("Read Error.");

    LogOutput::log() << "Read civ view map." << endl; 
}

// Saves the CivView to a .SAV file. Assumes os is already at the correct offset.
void Civ2Map::saveCivViewMap(ostream& os) const
                                   throw (runtime_error)
{
    if (has_civ_view == false)
    {
        throw runtime_error("Cannot Save: No map loaded.");
    }

    if (civ_view_map == NULL)
    {
        throw runtime_error("Cannot Save: No map allocated.");
    }

    os.write(reinterpret_cast<const char *>(civ_view_map.get()),
            map_area * sizeof(unsigned char) * 7);
    if (!os)
        throw runtime_error("Write Error.");

    LogOutput::log() << "Wrote civ_view_map " << endl;
    
}


// Converts an X,Y coordinate to an offset within the terrain map. As in
// Civ2, Some x and y values are not valid (in particular, x + y must be
// even).
// If a conversion from offset to x,y coordinates is ever needed,
// it would work like this:
/*
    width = x_dimension/2;
    y = i /  width
    x = (i % width) * 2          if y is even
        ((i % width) * 2) + 1    if y is odd
*/

int Civ2Map::XYtoOffset(int x, int y) const
                              throw (runtime_error)
{
    if ((x + y) % 2 != 0)
        throw runtime_error("Invalid x,y coordinates, x+y must be even.");

    int offset = x/2 + (y * (x_dimension/2));

    if (offset <0 || offset >= map_area)
        throw runtime_error("Invalid x,y coordinates: out of bounds.");

    return offset;
}

// Converts an X,Y coordinate to an offset within the civ_view_map. As in
// Civ2, Some x and y values are not valid (in particular, x + y must be
// even).

int Civ2Map::XYtoCivViewOffset(int x, int y, Civilization c) const
                                     throw (runtime_error)
{
    // Do not accept barbarians, there is now CivView information stored for
    // them
    if (c == RED) throw runtime_error("Barbarians do not have civ view info.");

    // use the terrain_map offset, and adjust for civilization
    int offset = XYtoOffset(x, y);

    // C++ won't let us subtract down the value of an enumeration
    int civ = c;

    offset += (civ-1) * map_area;

    return offset;
}

// Adds Tuples to vector "out" for each square in the city radius around x, y
void Civ2Map::getCityRadius(int x, int y, vector<Tuple>& out) const
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
        if (temp.y < 0 || temp.y >= y_dimension) continue;

        // When checking for x being out of bounds, do wrapping if the world
        // is round
        if (temp.x < 0)
        {
            if (!flat_earth)
            {
                temp.x = x_dimension - (temp.x % x_dimension);
            }
            else continue;
        }
        if (temp.x >= x_dimension)
        {
            if (!flat_earth)
            {
                temp.x %= x_dimension;
            }
            else continue;
        }

        // Insert the resulting coordinates in the vector
        out.push_back(temp);
    }
}
