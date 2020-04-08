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
#include <iomanip>
#include "civ2sav.h"

/////////////////////// Civ2Map Constants ///////////////////////////////


const unsigned char RIVER_FLAG = 0x80;
const unsigned char NO_RESOURCE_FLAG = 0x40;
const unsigned char TERRAIN_TYPE_MASK = 0x3F;


/////////////////////// Civ2Map Constructor ////////////////////////////////

// Private constructor called only by Civ2SavedGame
Civ2Map::Civ2Map(int x_dim, int y_dim, int area, bool has_civ_view_map,
                 int ma_pos, bool fe, Civ2Rules& in_rules) throw (runtime_error)
: rules(in_rules)
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

    // Setup resource map. This is generated based on grassland/resource patterns
    // and is not directly contained in a saved game or map file
    initResourceMap();
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

Civ2TerrainType Civ2Map::getTerrainType(int x, int y) const throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x,y);

    const TerrainCell& c = terrain_map[offset];

    return (Civ2TerrainType)(c.terrainType & TERRAIN_TYPE_MASK);
}

// Sets the terrain type (e.g. mountain, ocean, etc) index for a given map
// square.
void Civ2Map::setTerrainType(int x, int y, Civ2TerrainType t) throw (runtime_error)
{
    if (terrain_map.isNull())
    {
        throw runtime_error("No map loaded");
    }

    int offset = XYtoOffset(x,y);

    TerrainCell& c = terrain_map[offset];

    unsigned char index = (unsigned char)t;

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
// terrain. Note this does not include the reduction due to a city being
// present nearby, which can be performed by the adjustFertility() method
void Civ2Map::calcFertility(int x, int y) throw (runtime_error)
{
    if (terrain_map.isNull()) throw runtime_error("No map loaded.");

    int offset = XYtoOffset(x, y);

    // Find the sum of the food, shields and trade in the square itself,
    // the inner ring of the city radius around the square, and the outer ring
    // of the city radius around the square
    float selfFood = 0.0;
    float selfShields = 0.0;
    float selfTrade = 0.0;

    float innerFood = 0.0;
    float innerShields = 0.0;
    float innerTrade = 0.0;

    float outerFood = 0.0;
    float outerShields = 0.0;
    float outerTrade = 0.0;

    float food = 0;
    float shields = 0;
    float trade = 0;

    // Amount mining/irrigation adds to the food/shields. These were
    // empircally derived from observing how Civ2 calculates fertility
    // Note these still apply if food/shields is 0
    static const float IRRIGATION_BONUS = (2.0 / 3.0);
    static const float MINING_BONUS = (0.5);

    Civ2TerrainRules& terrain_rules = rules.getTerrainRules(map_position);
    RingIterator i(x, y, *this);

    Civ2TerrainType terrain_type;

    // Loop while in city radius
    while (i.getDistance() < 3)
    {
        // Get the food, shields, and trade for this terrain type
        terrain_type = getTerrainType(i.getX(), i.getY());
        food = terrain_rules.getFood(terrain_type);
        trade = terrain_rules.getTrade(terrain_type);

        // Shields is always 1 if a grassland square has a shield, otherwise
        // it is always 0 (at least as far as the Civ2 fertility calculation
        // seems to care)
        if (terrain_type == GRASSLAND)
        {
           if (hasGrasslandShield(i.getX(), i.getY()))
           {
              shields = 1;
           }
           else
           { 
              shields = 0;
           }
        }
        else
        {
           shields=terrain_rules.getShields(terrain_type);
        }


        // Add irrigation/mining bonus
        // Note both can't apply at once, and mining takes priority
        if (terrain_rules.canBeMined(terrain_type))
        {
            shields += MINING_BONUS;
        }
        else if (terrain_rules.canBeIrrigated(terrain_type))
        {
            food += IRRIGATION_BONUS;
        }
    
    
        // Add the squares contributions to the appropriate totals
        switch (i.getDistance())
        {
            case 0:
            {
                selfFood += food;
                selfShields += shields;
                selfTrade += trade;
                break;
            }
            case 1:
            {
                innerFood += food;
                innerShields += shields;
                innerTrade += trade;
                break;
            }
            case 2:
            {
                outerFood += food;
                outerShields += shields;
                outerTrade += trade;
                break;
            }
            default:
            {
                throw runtime_error("Unknown distance in calcFertility! This should never happen...");
                break;
            }
        }
        // Move to the next square
        ++i;
    }

    // Now combine food based on weights of various distances from the center
    // square. These weights were also found empirically
    static const float SELF_WEIGHT = 4.0;
    static const float INNER_WEIGHT = 2.0;
    static const float OUTER_WEIGHT = 1.0;

    float combinedFood = (selfFood * SELF_WEIGHT) + 
                         (innerFood * INNER_WEIGHT) + 
                         (outerFood * OUTER_WEIGHT);

    float combinedShields = (selfShields * SELF_WEIGHT) + 
                            (innerShields * INNER_WEIGHT) + 
                            (outerShields * OUTER_WEIGHT);

    float combinedTrade = (selfTrade * SELF_WEIGHT) + 
                          (innerTrade * INNER_WEIGHT) + 
                          (outerTrade * OUTER_WEIGHT);

    // Combine these into a floating point fertility value
    static const float FOOD_WEIGHT = 3.0;
    static const float SHIELDS_WEIGHT = 2.0;
    static const float TRADE_WEIGHT = 1.0;
    static const float DIVISOR = 16.0;


    float float_fertility = (FOOD_WEIGHT * combinedFood) +
                            (SHIELDS_WEIGHT * combinedShields) + 
                            (TRADE_WEIGHT * combinedTrade);

    float_fertility /= DIVISOR;

    // Now round off to an integer fertility
    unsigned char int_fertility = (unsigned char) round(float_fertility);
/* Yey! Commented out debug code!
    if (x == 7 && y == 45)
    {
        cout << "self food: " << selfFood << endl;
        cout << "self shields: " << selfShields << endl;
        cout << "self trade: " << selfTrade << endl;

        cout << "inner food: " << innerFood << endl;
        cout << "inner shields: " << innerShields << endl;
        cout << "inner trade: " << innerTrade << endl;

        cout << "outer food: " << outerFood << endl;
        cout << "outer shields: " << outerShields << endl;
        cout << "outer trade: " << outerTrade << endl;

        cout << "combined food: " << combinedFood << endl;
        cout << "combined shields: " << combinedShields << endl;
        cout << "combined trade: " << combinedTrade << endl;

        cout << "float/int fertility: " << setprecision(4) << float_fertility << "/" << (int)int_fertility << endl;
    }
*/
    // Another special feature of grassland squares. If they don't have 
    // a shield, their fertility is decremented by 1
    if (getTerrainType(x,y) == GRASSLAND && !hasGrasslandShield(x,y)) int_fertility --;

    // Fertility is not allowed to be less than 8 or more than 15
    // Note that effects of being in a city radius are handled by adjustFertility
    if (int_fertility <8) int_fertility = 8;
    else if (int_fertility > 15) int_fertility = 15;

    terrain_map[offset].fert_ownership = 
        (terrain_map[offset].fert_ownership & 0xF0) | int_fertility;
}

// Adjusts the fertility of a given square so that it is in the range
// of 0-7 if it is near a city.  Note that "near" a city does not mean 
// the city radius, rather a ring of radius 3 (with 5 squares at an edge
// rather than 3).
void Civ2Map::adjustFertility(int x, int y) throw (runtime_error)
{
    if (terrain_map.isNull()) throw runtime_error("No map loaded.");

    int offset = XYtoOffset(x, y);

    // It appears that this adjust ment is not the city radius, but one more ring
    // outside of that
    static const int MAX_ADJUST_RADIUS = 3;
    bool inCityRadius = false;

    for (RingIterator i(x, y, *this);
         i.getDistance() <= MAX_ADJUST_RADIUS; ++i)
    {
        if (getImprovements(i.getX(), i.getY()).hasCity()) inCityRadius = true;
    }
    unsigned char f = terrain_map[offset].fert_ownership & 0x0F;

    if (inCityRadius) 
    {
        // If another city has decremented fertility to below 8, then donot
        // decrement it again.
        if (f > 7) f-=8;
    }
 
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

// Returns whether a given map square has a grassland shield.
// This always returns false if the square is not a grassland square
bool Civ2Map::hasGrasslandShield(int x, int y) throw (runtime_error)
{
    if (getTerrainType(x, y) != GRASSLAND) return false;

    // It is a grassland, check the resource map
    int offset = XYtoOffset(x, y);

    if (resource_map[offset] & GRASS_SHIELD_FLAG) return true;
    else return false;
}

// Return whether the map is flat
bool Civ2Map::isFlat() throw (runtime_error)
{
    return flat_earth;
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

    LogOutput::log(DEBUG) << "Read terrain map" << endl;
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

    LogOutput::log(DEBUG) << "Wrote terrain map." << endl;
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

    LogOutput::log(DEBUG) << "Read civ view map." << endl; 
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

    LogOutput::log(DEBUG) << "Wrote civ_view_map " << endl;
    
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

// Initializes the resource map, which is an internal structure that
// maintains whether or not a square has a resource or grassland shield.
// Note this structures ignores whether the square is a grassland, or whether
// the resource has been supressed; it is used only to hold the patterns that
// Civ2 uses to determine these things.
// Currently only does the grassland shield pattern.
void Civ2Map::initResourceMap() throw (runtime_error)
{
    int horizontalSeed = 0;
    int evenVerticalSeed = 0;
    int oddVerticalSeed = 2;

    resource_map = new unsigned char[x_dimension * y_dimension];
    if (resource_map.isNull()) 
        throw new runtime_error ("Could not allocate enough memory for resource_map.");

    for (int y = 0; y < getHeight(); y++)
    {
        if (y % 2 == 0) // Even row
        {
            horizontalSeed = evenVerticalSeed;
            evenVerticalSeed+=3;
        }
        else
        {
            horizontalSeed = oddVerticalSeed;
            oddVerticalSeed +=3;
        }
           
        for (int x = y % 2; x < getWidth(); x+=2)
        {
            int offset = XYtoOffset(x,y);
            if ( (horizontalSeed / 2) % 2 == 0)
            {
                resource_map[offset] |= GRASS_SHIELD_FLAG;
            }
            else
            {
                resource_map[offset] &= (~GRASS_SHIELD_FLAG);
            }
            horizontalSeed ++;
        }
    }
}

///////////////////////// RingIterator Methods ////////////////////////////////

// Ring Iterator iterates through the squares adjacent to a square in a ring
// pattern. It starts out at the intialized point, moves up to the next ring,
// and then clock wise: Like so:
 
//                         19  20 9
//                      18  7  8  1 10 
//                      17  6  0  2 11
//                      16  5  4  3 12
//                         15 14 13
// Note that corners of the outer most ring are not included, except for the
// inner most ring. This is to be consistent with the way Civ 2 
// computes a city radius. (The above diagram is slighly tilted from
// the Civ2 Isometric view, think of the above shape as a city radius. So
// up in this view is really northwest in the Civ2 isometric view).

// The iterator also maintains a distance from the center point, which is 0
// at the center point, 1 in the first ring, 2 in the second ring and so on.

// Constructor, set up a RingIterator with a given point as its center
Civ2Map::RingIterator::RingIterator(int x, int y, Civ2Map& m)
 : map(m)
{
    centerX = x;
    centerY = y;
    reset();
}

// Copy constructor
Civ2Map::RingIterator::RingIterator(const RingIterator& ri)
 : map(ri.map)
{
    centerX = ri.centerX;
    centerY = ri.centerY;
    endRingX = ri.endRingX;
    endRingY = ri.endRingY;
    curX = ri.curX;
    curY = ri.curY;
    distance = ri.distance;
    direction = ri.direction;
    atCorner = ri.atCorner;
}

// Resets the iterator to the center point
void Civ2Map::RingIterator::reset()
{
    curX = centerX;
    curY = centerY;
    endRingX = centerX;
    endRingY = centerY;
    distance = 0;
    direction = 0;
    atCorner = false;
}

// Accessors for current position and distance
int Civ2Map::RingIterator::getX() { return curX; }
int Civ2Map::RingIterator::getY() { return curY; }
int Civ2Map::RingIterator::getDistance() { return distance; }

// Move to the next point in the ring pattern
void Civ2Map::RingIterator::moveToNextPoint() throw (runtime_error)
{
    // Loop until a point on the map is found
    bool offMap = true;
    bool startRingOffMap = false;  // True if this ring started off the map
    bool foundOnMap = true; // True if a point in this ring was on the map
    while (offMap)
    {
        // If we're at the end of a ring, time to move to the next ring
        // Note the end of the ring is always the top of the ring.
        if (endRingX == curX && endRingY == curY)
        {
            distance++;

            // Move top of ring to the top of the next ring
            // Not the direction: 0 = right, 1 = down, 2 = left, 3 = up
            if (movePoint(endRingX, endRingY,3, 1))
            {
                // We're starting at a square that's off the map, that's okay
                // if we're near the poles, but it's important to check for
                // a ring that's entirely off the map. That's what startRingOffMap
                // and foundOnMap are for

                if (startRingOffMap == true && foundOnMap == false)
                {
                    // The last ring started off the map, and never found a
                    // square on the map, so we've gone past the edge of the map
                    // That puts this iterator into an infinite loop, so throw
                    // an exception 
                    throw runtime_error("RingIterator went completely off map.");
                }
                // otherwise, keep a note that this ring started off the map 
                startRingOffMap = true;
            }
            else
            {
                startRingOffMap = false;
            }

            // Move current point to the square right of the end of the ring
            // For the inner ring, this includes the conrer point, just like Civ2
            direction = 0;
            curX = endRingX;
            curY = endRingY;
            offMap = movePoint(curX, curY, direction, 1);
            if (distance == 1) 
            {
                // We moved to the conrer point of the inner ring,
                // the next direction should be down (or up)
                direction++;
            }
        }
        else // Not at the end of the ring, move along this ring
        {
            // Check for being in a gap left by an inner ring
            // In this case reverse direction to go outside of the gap
            if (atCorner)
            {
                // Moving forwards twice in the direction yeilds a direction
                // in the opposite direction
                direction+=2;
                if (direction > 3) direction %= 4;
                
                offMap = movePoint(curX, curY, direction, 1);

                // Now move to the next direction to continue going along the ring
                direction++;
                if (direction > 3) direction = 0;

                atCorner = false;
            }
            else // Not in a gap
            {
                offMap = movePoint(curX, curY, direction, 1);

                // Check for a corner point
                if (curX == centerX || curY == centerY)
                {
                    // We just entered the  corner, change direction
                    direction++;
                    if (direction > 3) direction = 0;

                    // Skip over corner points (note that for the inner ring it wll be included)
                    if (distance != 1)
                    {
                        offMap = movePoint(curX, curY, direction, 1);

                        // If we're at distance > 2, there's a gap from the inside ring to fill
                        if (distance > 2)
                        {
                           direction++;
                           if (direction > 3) direction = 0;
                           offMap = movePoint(curX, curY, direction, 1);
                           atCorner = true; // signals for the next call to move back out of the gap.
                        }
                    }
                }// end if at a corner point
            } // end if in gap
        } // end if at end of ring

        if (!offMap) foundOnMap = true;

    } // end while(offMap)
}

// Moves a given x,y point a given distance in a given direction. It will
// perform the neccessary wrapping across the "dateline" at x = 0
// (if this is not a flat earth).
//  It will also return true if the point is off the map
// (by either being past the poles in the y axis, or past a flat earth in 
// the x axis).
// It takes a reference to the x, y coordinates to move, and a direction
// (0 = right, 1 = down, 2 = left, 3 = up)
bool Civ2Map::RingIterator::movePoint(int &x, int&y, const int& direction, const int &distance)
{
    static const int deltaX[4] = { 1,  1, -1, -1};
    static const int deltaY[4] = {-1,  1,  1, -1};

    // Move the point
    x += (distance * deltaX[direction]);
    y += (distance * deltaY[direction]);

    // Wrap the point
    bool offMap = false;
    if (x < 0 ||
        ((y%2 == 1) && x <1) ) // If y is odd, x must start at 1
    {
        if (!map.isFlat())
        {
            x = map.getWidth() - ((-x) % map.getWidth());
        }
        else offMap = true;
    }
    if (x >= map.getWidth() ||
        ( (y%2 == 0) && x == (map.getWidth()-1)  ) ) // If y is even x starts at 0, and so cannot reach mapWidth -1
    {
        if (!map.isFlat())
        {
            x %= map.getWidth();
        }
        else offMap = true;
    }
    if (y < 0 || y >= map.getHeight()) offMap = true; 
    return offMap;
}
