#include <fstream>
#include <string>
#include "civ2sav.h"


// test driver for test 7, sets all terrain to irrigated pollution, to
// make sure bitfields are in the right order

int main(int argc, char *argv[])
{

    Civ2SavedGame file;

    Civ2SavedGame::Improvements i;
    i.unit_present = 0;
    i.city_present = 0;
    i.irrigation = 1;
    i.mining = 0;
    i.road = 0;
    i.railroad = 0;
    i.fortress = 0;
    i.pollution = 1;

    Civ2SavedGame::WhichCivs w;
    w.red =    0;
    w.white =  1;
    w.green =  0;
    w.blue  =  0;
    w.yellow = 0;
    w.cyan =   0;
    w.orange = 0;
    w.purple = 0;

    try
    {

        file.load(argv[1]);
        file.setVerbose(true);

        for (int y = 0; y < file.getHeight(); y++)
        {
            for (int x = y % 2; x < file.getWidth(); x+=2)
            {
                file.setImprovements(x, y, i);
                file.setCivView(x, y, Civ2SavedGame::ALL, i);
                file.setVisibility(x, y, w);
            }
        }
        file.save(argv[1]);
    }
    catch(exception& e)
    {
        cout << "Exception: " << e.what() << endl;
        return 1;
    }
    catch(...)
    {
        cout << "Unknown error!\n";
        return 1;
    }
    return 0;
}

