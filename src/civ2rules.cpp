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
 * The Original Code is civ2rules, a utility class for accessing Civ2 rules.txt files.
 * 
 * The Initial Developer of the Original Code is James Dustin Reichwein.
 * Portions created by James Dustin Reichwein are
 * Copyright (C) 2005, James Dustin Reichwein.  All
 * Rights Reserved.
 * 
 * Contributor(s):
 */

// civ2rules.cpp
// Description:  Contains class for encapsulatng Civ2 rules.txt data
//
// Creation Date: Mar/26/2005
//
// Created By: Dusty Reichwein
//
// Revision History:
#include <iostream>

#include "civ2sav.h"


/////////////////////// Civ2Rules Methods ////////////////////////////////

Civ2Rules::Civ2Rules()
{
    // Create 4 terrain rules, one for each map. These use the default rules
    // from standard Civ2.
    map_terrain_rules.resize(4);
}

// Get the terrain rules for a given map number
Civ2TerrainRules& Civ2Rules::getTerrainRules(int mapNum) throw (runtime_error)
{
    if (mapNum >= map_terrain_rules.size()) 
    {
        stringstream message;
        message << "No terrain rules for map " << mapNum << " are defined.";
        throw new runtime_error(message.str());
    }

    return map_terrain_rules[mapNum];
}


///////////////////// Civ2TerrainRules constatants //////////////////////////////
// Default values for terrain rules
const Civ2TerrainRules::TerrainInfo Civ2TerrainRules::default_rules[NUM_TERRAIN_TYPES] =
{
//    Food, Shields, Trade, Allows Irrigation, Allows Mining
    { 0,    1,       0,     true,              true  }, // Desert
    { 1,    1,       0,     true,              false }, // Plains
    { 2,    1,       0,     true,              false }, // Grassland
    { 1,    2,       0,     false,             false }, // Forrest
    { 1,    0,       0,     true,              true  }, // Hills
    { 0,    1,       0,     false,             true  }, // Mountains
    { 1,    0,       0,     true,              false }, // Tundra
    { 0,    0,       0,     false,             true  }, // Glacier
    { 1,    0,       0,     false,             false }, // Swamp
    { 1,    0,       0,     false,             false }, // Jungle
    { 1,    0,       2,     false,             false }  // Ocean
};

/////////////////////// Civ2TerrainRules Methods ////////////////////////////////

// Construct a terrain type with the default rules
Civ2TerrainRules::Civ2TerrainRules()
{
    // Stupid ISO C++ wont' let me assign arrays, grumble grumble...
    for (int i = 0; i < NUM_TERRAIN_TYPES; i++)
    {
        terrain_rules[i] = default_rules[i];
    }
}
// end default constructor

// Return food production for a given terrain type
int Civ2TerrainRules::getFood(const Civ2TerrainType& t) const
{
    return terrain_rules[t].food;
}

// Return shield production for a given terrain type
int Civ2TerrainRules::getShields(const Civ2TerrainType& t) const
{
    return terrain_rules[t].shields;
}

// Return trade production for a given terrain type
int Civ2TerrainRules::getTrade(const Civ2TerrainType& t) const
{
    return terrain_rules[t].trade;
}

// Return whether a given terrain type can be irrigated
bool Civ2TerrainRules::canBeIrrigated(const Civ2TerrainType& t) const
{
    return terrain_rules[t].canBeIrrigated;
}

// Return whether a given terrain type can be mined
bool Civ2TerrainRules::canBeMined(const Civ2TerrainType& t) const
{
    return terrain_rules[t].canBeMined;
}
