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
// Feb/10/2005 JDR  Changes for mingw compiler. Specifically, added new classes
//                  Improvements and WhichCivs to avoid using non-portable 
//                  bitfield members. Changed fertility/ownership to one
//                  field to avoid using bitfields. Made fstream open
//                  parameters comply with C++ standard.
// Feb/20/2005 JDR  1.2Beta1 release of ToT multi-map support

#ifndef CIV2SAV_H_
#define CIV2SAV_H_

#include <string>
#include <stdexcept>
#include <vector>
#include <fstream>
#include "DustyUtil.h"

using namespace std;
using namespace DustyUtil;

// Improvements
// This class is encapsulates the Improvements byte field in Civ 2 saved games.
// Its intent is to hide the detail of the byte field's format from clients,
// while still keeping efficiency when copying (i.e., not splitting it up
// into individual booleans).
class Improvements
{
    public:
    Improvements();
    Improvements(const Improvements& i);
    Improvements& operator = (const Improvements& i);
    
    bool hasUnit();
    bool hasCity();
    bool hasIrrigation();
    bool hasMining();
    bool hasRoad();
    bool hasRailroad();
    bool hasFortress();
    bool hasPollution();
   
    void setUnit(bool b);
    void setCity(bool b);
    void setIrrigation(bool b);
    void setMining(bool b);
    void setRoad(bool b);
    void setRailroad(bool b);
    void setFortress(bool b);
    void setPollution(bool b);

    private:
    Improvements(unsigned char bitfield); // Called by friend Civ2SavedGame

    friend class Civ2Map;   // Since Civ2Map also has knowledge of
                            // the internal bit field format

    unsigned char improvements; // Actual byte from file       
      
    // Bit masks for setting/getting individual fields
    static const unsigned char UNIT_MASK;
    static const unsigned char CITY_MASK;
    static const unsigned char IRRIGATION_MASK;
    static const unsigned char MINING_MASK;
    static const unsigned char ROAD_MASK;
    static const unsigned char RAILROAD_MASK;
    static const unsigned char FORTRESS_MASK;
    static const unsigned char POLLUTION_MASK;
};
    
// WhichCivs
// This class is encapsulates the byte fields representing which
// civilizations own or have access to something in Civ 2 saved games.
// Its intent is to hide the detail of the byte field's format from clients,
// while still keeping efficiency when copying (i.e., not splitting it up
// into individual booleans).
class WhichCivs
{
    public:
    WhichCivs();
    WhichCivs(const WhichCivs& wc);
    WhichCivs& operator = (const WhichCivs& wc);
    
    bool hasRed();    // Barbarians
    bool hasWhite();  // Normally Romans, Russians, Celts
    bool hasGreen();  // Normally Babylonians, Zulus, Japanese
    bool hasBlue();   // Normally Germans, French, Vikings
    bool hasYellow(); // Normally Egyptipns, Aztecs, Spanish
    bool hasCyan();   // Normally Americans, Chinese, Persians
    bool hasOrange(); // Normally Greeks, English, Carthaginians
    bool hasPurple(); // Normally Indians, Mongols, Sioux
   
    void setRed(bool b);
    void setWhite(bool b);
    void setGreen(bool b);
    void setBlue(bool b);
    void setYellow(bool b);
    void setCyan(bool b);
    void setOrange(bool b);
    void setPurple(bool b);

    private:
    WhichCivs(unsigned char bitfield); // Called by friend Civ2SavedGame

    friend class Civ2Map;   // Since Civ2Map also has knowledge of
                            // the internal bit field format

    unsigned char whichCivs; // Actual byte from file       
      
    // Bit masks for setting/getting individual fields
    static const unsigned char RED_MASK;
    static const unsigned char WHITE_MASK;
    static const unsigned char GREEN_MASK;
    static const unsigned char BLUE_MASK;
    static const unsigned char YELLOW_MASK;
    static const unsigned char CYAN_MASK;
    static const unsigned char ORANGE_MASK;
    static const unsigned char PURPLE_MASK;
};

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
        void createSAV(int width, int height, int num_maps = 0) throw (runtime_error);

        int getWidth() const throw (runtime_error);
        int getHeight() const throw (runtime_error);

        unsigned short getSeed() const throw (runtime_error);
        void setSeed(unsigned short seed) throw (runtime_error);

        int getNumMaps() const throw (runtime_error);
        void addMap(int n) throw (runtime_error);
        void removeMap(int n) throw (runtime_error);

        Civ2Map& getMap(int n) throw (runtime_error);

        struct StartPositions
        {
            short x_positions[21];
            short y_positions[21];
        };

        StartPositions& getCivStart() throw (runtime_error);
        void setCivStart(const StartPositions& sp) throw (runtime_error);

        static bool isMPFile(string filename);

        bool supportsMultiMaps() const;


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


		// MERCATOR
		// Added loadMapHeaderOffset function declaration
		void loadMapHeaderOffset(istream& is)
			throw (runtime_error);
        void loadMapHeader(istream& is)
            throw(runtime_error);
        void saveMapHeader(ostream& os) const
            throw(runtime_error);

        unsigned short loadMapSpecificSeed(istream& is) throw (runtime_error);
        void saveMapSpecificSeed(ostream& os, unsigned short seed) throw (runtime_error);

        void loadStartPositions(istream& is) throw (runtime_error);
        void saveStartPositions(ostream& os) const throw (runtime_error);

        void destroyMaps();

        SmartPointer<MapHeader> header;
        SmartPointer<StartPositions> start_positions;
        vector<Civ2Map*> maps;

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

// Civ2Map
// This class encapsulates a single map in a Civ 2 games.
// For TOT saved games, there could be multiple maps per saved game.
// Civ2SavedGame is a friend of Civ2Map, and only Civ2SavedGame is allowed
// to construct a Civ2Map object.
class Civ2Map
{
    public:

        // Numeric values for default CIV2 terrain types
        enum { DESSERT=0, PLAINS, GRASSLAND, FOREST, HILLS, MOUNTAINS, TUNDRA,
               GLACIER, SWAMP,  JUNGLE, OCEAN };

        enum Civilization { RED=0, WHITE, GREEN, BLUE, YELLOW, CYAN, ORANGE, PURPLE,
                            ALL };

        void load(istream& is) throw (runtime_error);
        void save(ostream& os) throw (runtime_error);

        int getWidth() const throw (runtime_error);
        int getHeight() const throw (runtime_error);

        unsigned short getSeed() const throw (runtime_error);
        void setSeed(unsigned short seed) throw (runtime_error);

        Improvements getImprovements(int x, int y) const throw (runtime_error);
        void setImprovements(int x, int y, Improvements i) throw (runtime_error);

        WhichCivs getVisibility(int x, int y) const throw (runtime_error);
        void setVisibility(int x, int y, WhichCivs c) throw (runtime_error);

        unsigned char getBodyCounter(int x, int y) const throw (runtime_error);
        void setBodyCounter(int x, int y, unsigned char bc) throw (runtime_error);

        Civilization getCityRadius(int x, int y) const throw (runtime_error);
        void setCityRadius(int x, int y, Civilization c) throw (runtime_error);

        Improvements getCivView(int x, int y, Civilization c) const throw (runtime_error);
        void setCivView(int x, int y, Civilization c, Improvements i) throw (runtime_error);

        int getTypeIndex(int x, int y) const throw(runtime_error);
        void setTypeIndex(int x, int y, int i) throw(runtime_error);

        bool isRiver(int x, int y) const throw (runtime_error);
        void setRiver(int x, int y, bool river) throw(runtime_error);
        bool isResourceHidden(int x, int y) const throw(runtime_error);
        void setResourceHidden(int x, int y, bool hidden) throw(runtime_error);

        unsigned char getFertility(int x, int y) const throw (runtime_error);
        void setFertility(int x, int y, unsigned char) throw (runtime_error);
        void calcFertility(int x, int y) throw (runtime_error);

        void adjustFertility(int x, int y) throw (runtime_error);

        Civilization getOwnership(int x, int y) const throw (runtime_error);
        void setOwnership(int x, int y, Civilization civ) throw (runtime_error);

    private:
        struct TerrainCell
        {
            unsigned char terrainType;
            unsigned char improvements;
            unsigned char city_radius;
            unsigned char body_counter;
            unsigned char visibility;
            unsigned char fert_ownership; // upper nibble ownership
                                          // lower nibble fertility

            TerrainCell() 
            {
                terrainType = OCEAN; 
                improvements = 0;
                city_radius = 0;
                body_counter = 0;
                visibility = 0;
                fert_ownership = 0xF0;
            } 
                        

        };
        struct Tuple
        {
            int x;
            int y;
            Tuple(int a, int b) { x=a; y=b; }
            Tuple() {x=0; y=0;}
        };

        friend class Civ2SavedGame;    // Civ2Saved game is responsible for creating/
                                 // destroying Civ2Maps.

        Civ2Map(int x_dim, int y_dim, int area, bool has_civ_view_map,
                int mapPos, bool flat_earth) throw (runtime_error); 

        void loadCivViewMap(istream& is) throw (runtime_error);
        void saveCivViewMap(ostream& os) const throw (runtime_error);

        void loadTerrainMap(istream& is) throw(runtime_error);
        void saveTerrainMap(ostream& os) const throw(runtime_error);

        int XYtoOffset(int x, int y) const throw (runtime_error);

        int XYtoCivViewOffset(int x, int y, Civilization c) const throw (runtime_error);

        void getCityRadius(int x, int y, vector<Tuple>& out) const throw (runtime_error);

        SmartPointer<TerrainCell,true> terrain_map;
        SmartPointer<unsigned char,true> civ_view_map;

        // Fields from map header used by Civ2Map
        int x_dimension;
        int y_dimension;
        int map_area;
        bool flat_earth;

        // Whether or not a Civilization specific view map is needed
        bool has_civ_view;

        // Seed determining resource placement
        unsigned short int map_seed;

        // What position the map is within its saved game file
        unsigned char map_position;
};
#endif

