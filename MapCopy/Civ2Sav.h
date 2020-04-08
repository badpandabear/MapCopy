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

// civ2sav.h
// Description:  Contains class for reading Civ 2 saved game files.
//
// Creation Date: Aug/13/2000
//
// Created By: Dusty Reichwein
//
// Revision History:
// Sep/13/2000  JDR   Initial 1.Beta1 version.

#ifndef CIV2SAV_H_
#define CIV2SAV_H_

#include <string>
#include <stdexcept>
#include <vector>
#include <fstream>

#include <DustyUtil.h>

using namespace std;
using namespace DustyUtil;



// Civ2SavedGame
// This class is responsible for reading a Civ 2 saved game into memory,
// allowing the game to be modified, and writing it to back to a file.
//
class Civ2SavedGame
{
    public:

        Civ2SavedGame();
        ~Civ2SavedGame();

        void load(const string& filename) throw (runtime_error);
        void save(const string& filename) throw (runtime_error);

        void createMP(int width, int height) throw (runtime_error);
        void createSAV(int width, int height) throw (runtime_error);

        int getWidth() const throw (runtime_error);
        int getHeight() const throw (runtime_error);

        void setVerbose(bool verbose);

        unsigned short getSeed() const throw (runtime_error);
        void setSeed(unsigned short seed) throw (runtime_error);

        struct Improvements
        {
            unsigned unit_present : 1;
            unsigned city_present : 1;
            unsigned irrigation   : 1;
            unsigned mining       : 1;
            unsigned road         : 1;
            unsigned railroad     : 1;
            unsigned fortress     : 1;
            unsigned pollution    : 1;
        };

        Improvements getImprovements(int x, int y) const throw (runtime_error);
        void setImprovements(int x, int y, Improvements i) throw (runtime_error);

        enum Civilization { RED=0, WHITE, GREEN, BLUE, YELLOW, CYAN, ORANGE, PURPLE,
                            ALL };

        struct WhichCivs
        {
            unsigned red    : 1;  // Barbarians
            unsigned white  : 1;  // Normally Romans, Russians, Celts
            unsigned green  : 1;  // Normally Babylonians, Zulus, Japanese
            unsigned blue   : 1;  // Normally Germans, French, Vikings,
            unsigned yellow : 1;  // Normally Egyptipns, Aztecs, Spanish
            unsigned cyan   : 1;  // Normally Americans, Chinese, Persians,
            unsigned orange : 1;  // Normally Greeks, English, Carthaginians
            unsigned purple : 1;  // Normally Indians, Mongols, Sioux
        };
        WhichCivs getVisibility(int x, int y) const throw (runtime_error);
        void setVisibility(int x, int y, WhichCivs c) throw (runtime_error);


        unsigned char getBodyCounter(int x, int y) const throw (runtime_error);
        void setBodyCounter(int x, int y, unsigned char bc) throw (runtime_error);

        Civilization getCityRadius(int x, int y) const throw (runtime_error);
        void setCityRadius(int x, int y, Civilization c) throw (runtime_error);

        Improvements getCivView(int x, int y, Civilization c) const throw (runtime_error);
        void setCivView(int x, int y, Civilization c, Improvements i) throw (runtime_error);

        // Numeric values for default CIV2 terrain types
        enum { DESSERT=0, PLAINS, GRASSLAND, FOREST, HILLS, MOUNTAINS, TUNDRA,
               GLACIER, SWAMP,  JUNGLE, OCEAN };

        int getTypeIndex(int x, int y) const throw(runtime_error);
        void setTypeIndex(int x, int y, int i) throw(runtime_error);

        bool isRiver(int x, int y) const throw (runtime_error);
        void setRiver(int x, int y, bool river) throw(runtime_error);
        bool isResourceHidden(int x, int y) const throw(runtime_error);
        void setResourceHidden(int x, int y, bool hidden) throw(runtime_error);

        struct StartPositions
        {
            short x_positions[21];
            short y_positions[21];
        };
        unsigned char getFertility(int x, int y) const throw (runtime_error);
        void setFertility(int x, int y, unsigned char) throw (runtime_error);
        void calcFertility(int x, int y) throw (runtime_error);

        void adjustFertility(int x, int y) throw (runtime_error);
        StartPositions& getCivStart() throw (runtime_error);
        void setCivStart(const StartPositions& sp) throw (runtime_error);

        Civilization getOwnership(int x, int y) const throw (runtime_error);
        void setOwnership(int x, int y, Civilization civ) throw (runtime_error);

        static bool isMPFile(string filename);

    private:
        struct MapHeader
        {
            unsigned short int x_dimension;
            unsigned short int y_dimension;
            unsigned short int map_area;
            unsigned short int flat_earth;
            unsigned short int map_seed;
            unsigned short int locator_x_dimension;
            unsigned short int locator_y_dimension;
        };

        struct TerrainCell
        {
            unsigned char terrainType;
            Improvements improvements;
            unsigned char city_radius;
            unsigned char body_counter;
            WhichCivs visibility;
            unsigned fertility : 4;
            unsigned ownership : 4;

        };
        struct Tuple
        {
            int x;
            int y;
            Tuple(int a, int b) { x=a; y=b; }
            Tuple() {x=0; y=0;}
        };

		// MERCATOR
		// Added loadMapHeaderOffset function declaration
		void loadMapHeaderOffset(istream& is)
			throw (runtime_error);
        void loadMapHeader(istream& is)
            throw(runtime_error);
        void saveMapHeader(ostream& os) const
            throw(runtime_error);

        void loadStartPositions(istream& is) throw (runtime_error);
        void saveStartPositions(ostream& os) const throw (runtime_error);
        void loadCivViewMap(istream& is) throw (runtime_error);
        void saveCivViewMap(ostream& os) const throw (runtime_error);

        void loadTerrainMap(istream& is) throw(runtime_error);
        void saveTerrainMap(ostream& os) const throw(runtime_error);

        int XYtoOffset(int x, int y) const throw (runtime_error);

        int XYtoCivViewOffset(int x, int y, Civilization c) const throw (runtime_error);

        void getCityRadius(int x, int y, vector<Tuple>& out) const throw (runtime_error);

        SmartPointer<MapHeader> header;
        SmartPointer<StartPositions> start_positions;

        SmartPointer<TerrainCell,true> terrain_map;
        SmartPointer<Improvements,true> civ_view_map;
        bool verbose;
        bool isMP;

		// MERCATOR
		// I added the following 3 variables to fully support all civ2 versions:
		// - version: The savegame version number
		// - map_header_offset: The offset isn't the same in all versions, nor
		//   is it a constant in the ToT savegames, so I have to use a variable.
		// - secondary_maps: The 8th map header value used only in ToT.
		unsigned short version;
		unsigned int map_header_offset;
		unsigned short secondary_maps;

};



#endif