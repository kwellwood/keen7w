#include <allegro.h>
using namespace std;
#include "engine.h"
#include "portal.h"

int Portal::NextID = 0;
vector<Portal> Portal::Portals;

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
 * void addDest(int destx, int desty)                                        *
 *   destx,desty: teleporter destination                                     *
 *                                                                           *
 * Sets the destination of the last portal in the collection. Used by TED    *
 * when placing the destination of a new portal.                             *
 * ------------------------------------------------------------------------- */
void Portal::addDest(int destx, int desty)
{
    Portals[Portals.size()-1]._destx = destx;
    Portals[Portals.size()-1]._desty = desty;
}

/* ------------------------------------------------------------------------- *
 * void deleteLast()                                                         *
 *                                                                           *
 * Removes the last portal added to the collection. Used by TED to cancel    *
 * added a new portal.                                                       *
 * ------------------------------------------------------------------------- */
void Portal::deleteLast()
{   if (Portals.size()> 0) Portals.pop_back(); }

/* ------------------------------------------------------------------------- *
 * void deleteLast(int p)                                                    *
 *   p: number of the portal in collection                                   *
 *                                                                           *
 * Deletes the given portal.                                                 *
 * ------------------------------------------------------------------------- */
void Portal::delPortal(int p)
{
    vector<Portal>::iterator iter = Portals.begin();
    while (p > 0) { iter++; p--; }
    Portals.erase(iter);
}

/* ------------------------------------------------------------------------- *
 * void readPortals(ifstream &file)                                          *
 *   file: file stream to read from                                          *
 *                                                                           *
 * Reads the portals from a map file.                                        *
 * ------------------------------------------------------------------------- */
void Portal::readPortals(ifstream &file)
{
    clearPortals();
    
    int numportals;
    if (MAP_FORMAT_IN == K7W) { numportals = readInt(file); }
    else { numportals = readqbInt(file); }
    
    for (int p=0; p<numportals; p++)
    {   int x, y, destx, desty;
        if (MAP_FORMAT_IN == K7W)
        {   x = readInt(file); y = readInt(file);
            destx = readInt(file); desty = readInt(file); }
        else
        {   x = readqbInt(file); y = readqbInt(file);
            destx = readqbInt(file); desty = readqbInt(file); }

        addPortal(x, y, destx, desty);
    }
}

/* ------------------------------------------------------------------------- *
 * static void drawPortals(float zoom, int vpx, int vpy)                     *
 *   zoom: current zoom factor of the camera                                 *
 *   vpx,vpy: viewport coordinates (used by TED)                             *
 *                                                                           *
 * Draws all of the portals in the collection.                               *
 * ------------------------------------------------------------------------- */
void Portal::drawPortals(float zoom, int vpx, int vpy)
{   for (int i=0; i<Portals.size(); i++) Portals[i].draw(i, zoom, vpx, vpy); }

/* ------------------------------------------------------------------------- *
 * void writeMap(ofstream &file)                                             *
 *   file: file stream to write to                                           *
 *                                                                           *
 * Writes the portals to an open map file; used by TED                       *
 * ------------------------------------------------------------------------- */
void Portal::writeMap(ofstream &file)
{
    int temp = Portals.size(); file.write((char*)&temp, 4);
    
    for (int i=0; i<Portals.size(); i++)
    {   file.write((char*)&Portals[i]._x, 4);
        file.write((char*)&Portals[i]._y, 4);
        file.write((char*)&Portals[i]._destx, 4);
        file.write((char*)&Portals[i]._desty, 4);
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
{   Portals.clear(); NextID = 0; }

/* ------------------------------------------------------------------------- *
 * Portal(int x, int y, int destx, int desty)                                *
 *   x,y: teleporter location                                                *
 *   destx,desty: teleporter destination                                     *
 *                                                                           *
 * The default constructor.                                                  *
 * ------------------------------------------------------------------------- */
Portal::Portal(int x, int y, int destx, int desty)
:_x(x), _y(y), _destx(destx), _desty(desty)
{ _id = NextID; NextID++; }

/* ------------------------------------------------------------------------- *
 * void draw(float zoom, int vpx, int vpy)                                   *
 *   zoom: current zoom factor of the camera                                 *
 *   vpx,vpy: viewport coordinates (used by TED)                             *
 *                                                                           *
 * Draws rectangles around the entry and exit of the portal.                 *
 * ------------------------------------------------------------------------- */
void Portal::draw(int id, float zoom, int vpx, int vpy)
{
    // --- Draw the source box ---
    int x1 = (int)(_x - (PORTALWIDTH/zoom)/2);
    int y1 = (int)(_y - (PORTALHEIGHT/zoom)/2);
    int x2 = (int)(_x + (PORTALWIDTH/zoom)/2);
    int y2 = (int)(_y + (PORTALHEIGHT/zoom)/2);

    if (onCamera((float)x1, (float)y1, (float)x2, (float)y2))
    {
        int x = (int)(_x/zoom - (int)getCameraOrigX() - (PORTALWIDTH/zoom/2));
        int y = (int)(_y/zoom - (int)getCameraOrigY() - (PORTALHEIGHT/zoom/2));

        drawBox(x+vpx, y+vpy,
            (int)(x+PORTALWIDTH/zoom+vpx), (int)(y+PORTALHEIGHT/zoom+vpy),
            144, EMPTY);
        if (zoom <= 1)
            drawText("P"+toStr(id),
                x + (int)(PORTALWIDTH/zoom/2+vpx) - 5,
                y + (int)(PORTALHEIGHT/zoom/2+vpy) - 3, 144);
    }

    // --- Draw the destination box ---
    int width = (int)((float)PORTALWIDTH*.7);
    int height = (int)((float)PORTALHEIGHT*.7);
    x1 = (int)(_destx - (width/zoom)/2);
    y1 = (int)(_desty - (height/zoom)/2);
    x2 = (int)(_destx + (width/zoom)/2);
    y2 = (int)(_desty + (height/zoom)/2);

    if (onCamera((float)x1, (float)y1, (float)x2, (float)y2))
    {
        int x = (int)(_destx/zoom - (int)getCameraOrigX() - (width/zoom/2));
        int y = (int)(_desty/zoom - (int)getCameraOrigY() - (height/zoom/2));

        drawBox(x+vpx, y+vpy,
            (int)(x+width/zoom+vpx), (int)(y+height/zoom+vpy),
            167, EMPTY);
        if (zoom <= 1)
            drawText("D"+toStr(id), (int)(x+width/zoom/2+vpx-5),
                (int)(y+height/zoom/2+vpy-3), 167);
    }
}

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

string Portal::toStr(int n)
{   char buffer[11]; return string(itoa(n, buffer, 10)); }

