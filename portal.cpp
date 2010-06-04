#include <allegro.h>
using namespace std;
#include "engine.h"
#include "portal.h"

/* ------------------------------------------------------------------------- *
 * void addPortal(int x, int y, int destx, int desty)                        *
 *   x,y: teleporter location                                                *
 *   destx,desty: teleporter destination                                     *
 *                                                                           *
 * Add a new portal to the collection.                                       *
 * ------------------------------------------------------------------------- */
void Portal::addPortal(int x, int y, int destx, int desty)
{   Portals.push_back(Portal(x, y, destx, desty)); }

/* ------------------------------------------------------------------------- *
 * void readPortals(ifstream &file)                                          *
 *   file: file stream to read from                                          *
 *                                                                           *
 * Reads the portals from a map file.                                        *
 * ------------------------------------------------------------------------- */
void Portal::readPortals(ifstream &file)
{
    Portals.clear();
    
    int numportals = readInt(file);
    for (int p=0; p<numportals; p++)
    {   int x = readInt(file);
        int y = readInt(file);
        int destx = readInt(file);
        int desty = readInt(file);

        addPortal(x, y, destx, desty);
    }
}

/* ------------------------------------------------------------------------- *
 * int atPortal(float x, float y)                                            *
 *   x,y: point to check                                                     *
 *                                                                           *
 * Checks if the given point is inside any of the portal boxes, and returns  *
 * the position in Portals of the first one found. Returns -1 if point is    *
 * not in any portal.                                                        *
 * ------------------------------------------------------------------------- */
int Portal::atPortal(float x, float y)
{
    for (int p=0; p<Portals.size(); p++)
    {   if (Portals[p].collision(x, y)) return p; }
    return -1;
}

/* ------------------------------------------------------------------------- *
 * Portal getPortal(int p)                                                   *
 *   p: position in Portals vector                                           *
 *                                                                           *
 * Returns a portal from the collection.                                     *
 * ------------------------------------------------------------------------- */
Portal Portal::getPortal(int p)
{   return Portals[p]; }

/* ------------------------------------------------------------------------- *
 * void clearPortals()                                                       *
 *                                                                           *
 * Clears all the portals in the collection.                                 *
 * ------------------------------------------------------------------------- */
void Portal::clearPortals()
{   Portals.clear(); }

/* ------------------------------------------------------------------------- *
 * Portal(int x, int y, int destx, int desty)                                *
 *   x,y: teleporter location                                                *
 *   destx,desty: teleporter destination                                     *
 *                                                                           *
 * The default constructor.                                                  *
 * ------------------------------------------------------------------------- */
Portal::Portal(int x, int y, int destx, int desty)
:_x(x), _y(y), _destx(destx), _desty(desty)
{ }

/* ------------------------------------------------------------------------- *
 * bool collision(float x, float y)                                          *
 *   x,y: point to check                                                     *
 *                                                                           *
 * Checks if a point is inside the bounding box of the portal.               *
 * ------------------------------------------------------------------------- */
bool Portal::collision(float x, float y)
{   if (x >= _x - PORTALWIDTH/2  && x <= _x + PORTALWIDTH/2 &&
        y >= _y - PORTALHEIGHT/2 && y <= _y + PORTALWIDTH/2) return true;
    return false;
}

int Portal::x()     { return _x;   }
int Portal::y()     { return _y;   }
int Portal::destx() { return _destx; }
int Portal::desty() { return _desty; }