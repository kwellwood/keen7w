#include <iostream>
#include <fstream>
#include <string>
#include <allegro.h>
using namespace std;
#include "engine.h"
#include "audio.h"
#include "graphicslib.h"
#include "shot.h"
#include "player.h"
#include "map.h"
#include "sprite.h"
#include "item.h"
#include "enemy.h"
#include "portal.h"
#include "lift.h"

/* ------------------------------------------------------------------------- *
 * Map()                                                                     *
 *   tilelib: the graphics library for this map                              *
 *                                                                           *
 * The default constructor.                                                  *
 * ------------------------------------------------------------------------- */
Map::Map(GraphicsLib* tilelib)
:_width(0), _height(0), _tilelib(tilelib), _morphtilesupdated(0)
{   _map = NULL; _mapFG = NULL; _switchTag = NULL;
    for (int i=0; i<25; i++) _switch[i]=false;
}

/* ------------------------------------------------------------------------- *
 * ~Map()                                                                    *
 *                                                                           *
 * The destructor.                                                           *
 * ------------------------------------------------------------------------- */
Map::~Map()
{
    delete _map;
    delete _mapFG;
    delete _switchTag;
    _morphtiles.clear();
}

/* ------------------------------------------------------------------------- *
 * bool load(string filename)                                                *
 *   filename: file to load                                                  *
 *                                                                           *
 * Loads a map file and returns true if successful.                          *
 * ------------------------------------------------------------------------- */
bool Map::load(string filename)
{
    // --- Delete previous arrays ---
    if (_map != NULL)
    {
        delete _map;       _map = NULL;
        delete _mapFG;     _mapFG = NULL;
        delete _switchTag; _switchTag = NULL;
    }

    ifstream file(filename.c_str(), ios::in|ios::binary);
    if (!file.is_open()) return false;

    // --- Verify file type ---
    char idtag[3];
    file.read(idtag, 3);
    if (idtag[0] != 'C' || idtag[1] != 'K' || idtag[2] != '7')
    {   file.close(); return false; }

    // --- Map dimensions ---
    _width = readInt(file);
    _height = readInt(file);
    if (_width <= 0 || _width >= 512 || _height <= 0 || _height >= 512)
    {   return false; }

    _map = new int[_height*_width];
    _mapFG = new int[_height*_width];
    _switchTag = new int[_height*_width];

    // --- Background music ---
    char music[12];
    file.read(music, 12); _music = string(music);
    _music = _music.substr(0, _music.find(" "));
    if (_music != "NONE" && _music != "")
    {   Audio::playMusic(_music); }
    else
    {   Audio::stopMusic(); }

    // --- Map tile data ---
    _morphtiles.clear();
    for (int ry=1; ry<=_height; ry++)
    {   for (int rx=1; rx<=_width; rx++)
        {
            setMap(rx, ry, readLong(file));             // background
            setMapFG(rx, ry, readLong(file));           // foreground
            setSwitchTag(rx, ry, readLong(file));       // switchs layer
            if (_tilelib->tileType2(mapFG(rx, ry)) == MORPHTILE)
            {   addMorphTile(rx, ry); }                 // a changing tile
        }
    }

    playerReadMap(file);
    Item::readItems(file);
    Sprite::clearSprites();
    Shot::clearShots();
    Enemy::readEnemies(file);
    Portal::readPortals(file);
    Lift::readLifts(file);

    // --- Reset switches ---
    for (int i=0; i<25; i++) _switch[i]=false;

    file.close();
    return true;
}

/* ------------------------------------------------------------------------- *
 * void tick()                                                               *
 *                                                                           *
 * Does everything the map needs to do on a single game cycle tick.          *
 * ------------------------------------------------------------------------- */
void Map::tick()
{
    updateMorphTiles();
}

/* ------------------------------------------------------------------------- *
 * void setMap(int x, int y, int t)                                          *
 *   x,y: location of tile                                                   *
 *   t  : tile number                                                        *
 *                                                                           *
 * Sets a location on the background layer of the map to a tile.             *
 * ------------------------------------------------------------------------- */
void Map::setMap(int x, int y, int t)
{   if (x > 0 && x <= _width && y > 0 && y <= _height)
    {   _map[(y * _width - _width) + x - 1] = t; }
}

/* ------------------------------------------------------------------------- *
 * bool load(string filename)                                                *
 *   x,y: location of tile                                                   *
 *   t  : tile number                                                        *
 *                                                                           * 
 * Sets a location on the foreground layer of the map to a tile.             *
 * ------------------------------------------------------------------------- */
void Map::setMapFG(int x, int y, int t)
{   if (x > 0 && x <= _width && y > 0 && y <= _height)
    {   _mapFG[(y * _width - _width) + x - 1] = t; }
}

/* ------------------------------------------------------------------------- *
 * int map(int x, int y)                                                     *
 *   x,y: location on map                                                    *
 *                                                                           * 
 * Returns the tile number at location x,y on the map background.            *
 * ------------------------------------------------------------------------- */
int Map::map(int x, int y)
{   if (x > 0 && x <= _width && y > 0 && y <= _height)
    {   return _map[(y * _width - _width) + x - 1]; }
    return 0;
}
/* ------------------------------------------------------------------------- *
 * int mapFG(int x, int y)                                                   *
 *   x,y: location on map                                                    *
 *                                                                           * 
 * Returns the tile number at location x,y on the map foreground.            *
 * ------------------------------------------------------------------------- */
int Map::mapFG(int x, int y)
{   if (x > 0 && x <= _width && y > 0 && y <= _height)
    {   return _mapFG[(y * _width - _width) + x - 1]; }
    return 0;
}

/* ------------------------------------------------------------------------- *
 * int height()                                                              *
 *                                                                           * 
 * Returns the height of the map.                                            *
 * ------------------------------------------------------------------------- */
int Map::height()
{   return _height; }

/* ------------------------------------------------------------------------- *
 * int width()                                                               *
 *                                                                           *
 * Returns the width of the map.                                             *
 * ------------------------------------------------------------------------- */
int Map::width()
{   return _width; }

/* ------------------------------------------------------------------------- *
 * void addMorphTile(int x, int y)                                           *
 *   x,y: location of the tile to add as a changing tile                     *
 *                                                                           *
 * Adds a location to the changing tiles vector. This is a collection of     *
 * tiles which change regularly and must be updated during gameplay.         *
 * ------------------------------------------------------------------------- */
void Map::addMorphTile(int x, int y)
{   _morphtiles.push_back(Coordinate(x,y)); }

void Map::updateMorphTiles()
{
    // dont update twice in same anmcounter clock cycle
    if (_morphtilesupdated == anmCounter()) return;

    for (int i=0; i<_morphtiles.size(); i++)
    {
        int x = _morphtiles[i].x, y = _morphtiles[i].y;
        int tile = mapFG(x, y);
        int nexttile = _tilelib->slopeMask(tile);
        int delay = _tilelib->anmLength(tile);
        
        if (anmCounter() % delay == 0) setMapFG(x, y, nexttile);
    }

    _morphtilesupdated = anmCounter();
}

/* ------------------------------------------------------------------------- *
 * int slopeHeight(int x, int y, int l)                                      *
 *   x,y: location on map (in pixels)                                        *
 *   l  : map layer (0=background, 1=foreground)                             *
 *                                                                           *
 * Takes absolute map coordinates in pixels and returns the height of the    *
 * slope of the file at that location in pixels. Heights range from 0 to     *
 * tilesize - 1.                                                             *
 * ------------------------------------------------------------------------- */
int Map::slopeHeight(int x, int y, int l)
{
    /*      Slope Types:
    -------------------------------
    01 - /.    05 - /.1    09 - \'1
    02 - .\    06 - /.2    10 - \'2
    03 - \'    07 - .\1    11 - '/1
    04 - '/    08 - .\2    12 - '/2 */

    int tx = (int)(x / ts) + 1;     // get tile coordinates
    int ty = (int)(y / ts) + 1;

    int tile=0;
    if (l == 0)      tile = map(tx, ty);    // get tile number from background
    else if (l == 1) tile = mapFG(tx, ty);  // or foreground
    int stype = _tilelib->slopeMask(tile);  // get slope type (see table)
    
    int px = (tx - 1) * ts;         // upper left corner of
    int py = (ty - 1) * ts;         // tile, in pixels
    int sx = x - px;            // difference between x,y and
    int sy = y - py;            // upper left of tile in pixels

    int hit=0;
    switch (stype)
    {
        case 1: if (sx > ts-sy-1) hit=(ts-sy-1)-sx; break;
        case 2: if (sx < sy) hit=sx-sy; break;
        case 3: if (sx > sy) hit=sx-sy; break;
        case 4: if (sx < ts-sy) hit=(ts-sy-1)-sx; break;
        case 5: if (sx > (ts-sy-1)*2) hit=(int)((ts-sy-1)-sx/2); break;
        case 6: if (sx > (ts-sy-1)*2-ts) hit=(int)((ts-sy-1)-ts/2-sx/2); break;
        case 7: if (sx < sy*2-ts) hit=(ts-sy-1)-(int)((ts-sx)/2-.5); break;
        case 8: if (sx < sy*2) hit=(ts-sy-1)-(int)((ts-sx)/2-.5)-ts/2; break;
        case 9: if (sx > sy*2+1) hit=1; break;
        case 10: if (sx > sy*2-(ts-1)) hit=1; break;
        case 11: if (sx < ts-(sy*2)-2) hit=1; break;
        case 12: if (sx < ts-(sy*2)+(ts-2)) hit=1; break;
    }
    return hit;
}

/* ------------------------------------------------------------------------- *
 * int hitPoint(float x, float y)                                            *
 *   x,y: location on map (in pixels)                                        *
 *                                                                           * 
 * Returns the type of tile at a location assuming the player collided       *
 * with it. The most important tile type of foreground or background is      *
 * returned.                                                                 *
 * ------------------------------------------------------------------------- */
int Map::hitPoint(float x, float y)
{
    // fg  = foreground tiletype
    // fg2 = foreground tiletype2
    // bg  = background tiletype
    
    // --- get tile coordinates ---
    int mx = (int)(x / ts) + 1;
    int my = (int)(y / ts) + 1;
    
    int fg = tileType(getMapFG(mx, my));
    int fg2 = tileType2(getMapFG(mx, my));
    int bg = tileType(getMap(mx, my));
    
    if (fg2 == ENDLEVEL) return ENDLEVEL;      // exit level tile
    if (fg2 == SCUBAGEAR) return SCUBAGEAR;    // get scuba gear tile
                                               // *** tagged as tiletype 11??
    
    if (fg == SLOPE) return FGSLOPE;           // slope in foreground
    if (fg == HAZZARD) return HAZZARD;         // hazzard in foreground
    if (fg == ONEWAYFLOOR) return ONEWAYFLOOR; // oneway floor in foreground
    if (fg == GEMSOCKET) return GEMSOCKET;     // gem socket in foreground
    if (fg == FGWALL) return WALL;             // wall in foreground

    return bg;                                 // all background tiletypes
}

/* ------------------------------------------------------------------------- *
 * void setSwitchTag(int x, int y, int sw)                                   *
 *   x,y: location on map                                                    *
 *   s  : switch tag                                                         * 
 *                                                                           * 
 * Sets a value at a location on the switch layer of the map.                *
 * ------------------------------------------------------------------------- */
void Map::setSwitchTag(int x, int y, int sw)
{   if (x > 0 && x <= _width && y > 0 && y <= _height)
    {   _switchTag[(y * _width - _width) + x - 1] = sw; }
}

/* ------------------------------------------------------------------------- *
 * int switchTag(int x, int y)                                               *
 *   x,y: location on map                                                    *
 *                                                                           *
 * Gets the value at a location on the switch layer of the map.              *
 * ------------------------------------------------------------------------- */
int Map::switchTag(int x, int y)
{   if (x > 0 && x <= _width && y > 0 && y <= _height)
    {   return _switchTag[(y * _width - _width) + x - 1]; }
    return 0;
}

/* ------------------------------------------------------------------------- *
 * void setSwitch(int s, bool v)                                             *
 *   s: switch number (1-25)                                                 *
 *   v: value for switch                                                     *
 *                                                                           *
 * Sets the value of a switch (valid switches are 1 to 25.)                  *
 * ------------------------------------------------------------------------- */
void Map::setSwitch(int s, bool v)
{   if (s > 0 && s <= 25) _switch[s-1] = v; }

/* ------------------------------------------------------------------------- *
 * bool getSwitch(int s)                                                     *
 *   s: switch                                                               *
 *                                                                           *
 * Returns the value of a switch (valid switches are 1 to 25.)               *
 * ------------------------------------------------------------------------- */
bool Map::getSwitch(int s)
{   if (s > 0 && s <= 25) return _switch[s-1]; return false; }

/* ------------------------------------------------------------------------- *
 * void toggleBridge(int loc)                                                *
 *   loc: single integer containing location of upper left of bridge         *
 *        (see code below)                                                   *
 *                                                                           *
 * Toggles the tiles of the bridge at the location given. SwitchTags are     *
 * used to determine the current status of the bridge.                       *
 * ------------------------------------------------------------------------- */
void Map::toggleBridge(int loc)
{
    // loc = (by * mapWidth - mapWidth) + bx
    int tileshift;
    int by = (int)(loc / _width) + 1;
    int bx = loc - (by * _width - _width);
    
    int bridgelen = switchTag(bx, by);
    int bridgeON = switchTag(bx, by+1);
    if (bridgeON == 1) tileshift = 1; else tileshift = -1;
    for (int x=bx; x<bx+bridgelen; x++)
    {   setMapFG(x, by, mapFG(x, by) + tileshift);
        setMapFG(x, by + 1, mapFG(x, by + 1) + tileshift);
    }
    
    if (bridgeON == 1) setSwitchTag(bx,by+1,0); else setSwitchTag(bx,by+1,1);
}

/* ------------------------------------------------------------------------- *
 * void unlockDoor(int mx, int my)                                           *
 *   mx,my: map location of the gem socket to be used                        *
 *                                                                           *
 * Changes the tiles of the door connected to the gem socket given so that   *
 * the door is open. The gem socket will also be filled.                     *
 * ------------------------------------------------------------------------- */
void Map::unlockDoor(int mx, int my)
{
    int loc = switchTag(mx, my);           // use switchtag to find the top of
    int dy = (int)(loc / _width) + 1;      // the door to open. check there to
    int dx = loc - (dy * _width - _width); // find the height of the door then
    int dh = switchTag(dx, dy);            // change all the tiles.
    if (dh <= 0) error("Door has no height");
    
    for (int y=dy; y<dy+dh; y++)
        setMapFG(dx, y, mapFG(dx, y) + 1);
    
    // switch tile to filled gem socket
    setMapFG(mx, my, mapFG(mx, my) + 1);
    
    // clear up switchtags
    setSwitchTag(mx, my, 0);
    setSwitchTag(dx, dy, 0);
}