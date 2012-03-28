#include <iostream>
#include <fstream>
#include <allegro.h>
using namespace std;
#include "engine.h"
#include "audio.h"
#include "sprite.h"
#include "item.h"

/* ------------------------------------------------------------------------- *
 * static void addItem(Item itm)                                             *
 *                                                                           *
 * Adds a new item into the collection.                                      *
 * ------------------------------------------------------------------------- */
void Item::addItem(Item* itm)
{   Items.push_back(itm); }

/* ------------------------------------------------------------------------- *
 * static void killItem(int i)                                               *
 *   i: item's position in the Items vector                                  *
 *                                                                           *
 * Gives the item's bonus to the player and removes it from the collection.  *
 * ------------------------------------------------------------------------- */
void Item::killItem(int i)
{
    Items[i]->giveBonus();

    delete Items[i];
    vector<Item*>::iterator iter = Items.begin();
    while (i > 0) { iter++; i--; }
    Items.erase(iter);
}

/* ------------------------------------------------------------------------- *
 * static void delItem(int i)                                                *
 *   i: item's position in the Items vector                                  *
 *                                                                           *
 * Only removes an item from the collection. Like killItem but without       *
 * giving any bonus.                                                         *
 * ------------------------------------------------------------------------- */
void Item::delItem(int i)
{
    delete Items[i];
    vector<Item*>::iterator iter = Items.begin();
    while (i > 0) { iter++; i--; }
    Items.erase(iter);    
}

/* ------------------------------------------------------------------------- *
 * static void readItems(ifstream &file)                                     *
 *                                                                           *
 * Reads in a list of items from a map file.                                 *
 * ------------------------------------------------------------------------- */
void Item::readItems(ifstream &file)
{
    clearItems();

    int numitems;
    if (MAP_FORMAT_IN == K7W) { file.read((char*)&numitems, 4); }
    else { numitems = readqbInt(file); }
    
    for (int i=0; i<numitems; i++)
    {   int x, y, id;
        if (MAP_FORMAT_IN == K7W)
        {   file.read((char*)&x, 4);
            file.read((char*)&y, 4);
            file.read((char*)&id, 4);
        }
        else
        {   x = readqbInt(file);
            y = readqbInt(file);
            id = readqbInt(file);
        }
        
        if (id == SODA)           addItem(new Soda(x, y));
        else if (id == GUM)       addItem(new Gum(x, y));
        else if (id == CHOCOLATE) addItem(new Chocolate(x, y));
        else if (id == CANDY)     addItem(new Candy(x, y));
        else if (id == DONUT)     addItem(new Donut(x, y));
        else if (id == ICECREAM)  addItem(new Icecream(x, y));
        else if (id == FLASK)     addItem(new Flask(x, y));
        else if (id == GUN)       addItem(new Gun(x, y));
        else if (id == DROP)      addItem(new Drop(x, y));
        else if (id == REDKEY)    addItem(new Redkey(x, y));
        else if (id == BLUKEY)    addItem(new Blukey(x, y));
        else if (id == GRNKEY)    addItem(new Grnkey(x, y));
        else if (id == YELKEY)    addItem(new Yelkey(x, y));
        else error("Unrecognized item type");
    }
}

/* ------------------------------------------------------------------------- *
 * static void writeMap(ofstream &file)                                      *
 *   file: file stream to write to                                           *
 *                                                                           *
 * Writes out all the items for a map file.                                  *
 * ------------------------------------------------------------------------- */
void Item::writeMap(ofstream &file)
{
    writeInt(file, Items.size());
    for (int i=0; i<Items.size(); i++)
    {   writeInt(file, Items[i]->_x);
        writeInt(file, Items[i]->_y);
        writeInt(file, Items[i]->_id);
    }
}

/* ------------------------------------------------------------------------- *
 * static void drawItems()                                                   *
 *                                                                           *
 * Draws all of the items in the collection.                                 *
 * ------------------------------------------------------------------------- */
void Item::drawItems()
{   for (int i=0; i<Items.size(); i++) Items[i]->draw(1, 0, 0); }

/* ------------------------------------------------------------------------- *
 * static int collideItem(int x1, int y1, int x2, int y2)                    *
 *   x1,y1,x2,y2: corners of the collision rectangle                         *
 *                                                                           *
 * Searchs the collection and returns position of the first item which       *
 * intersects the given collision rectangle. Returns -1 if no collisions.    *
 * ------------------------------------------------------------------------- */
int Item::collideItem(int x1, int y1, int x2, int y2)
{
    for (int i=0; i<Items.size(); i++)
    {   if (Items[i]->collision(x1, y1, x2, y2)) return i; }

    return -1;
}

/* ------------------------------------------------------------------------- *
 * static void clearItems()                                                  *
 *                                                                           *
 * Erases all items.                                                         *
 * ------------------------------------------------------------------------- */
void Item::clearItems()
{   for (int i=0; i<Items.size(); i++) delete Items[i]; Items.clear(); }

/* ------------------------------------------------------------------------- *
 * static void writeSave(ofstream &file)                                     *
 *   file: file stream to write to                                           *
 *                                                                           *
 * Writes out all the items for a saved game file.                           *
 * ------------------------------------------------------------------------- */
void Item::writeSave(ofstream &file)
{
    writeInt(file, Items.size());
    
    for (int i=0; i<Items.size(); i++)
    {
        writeInt(file, Items[i]->_id);
        writeInt(file, Items[i]->_x);
        writeInt(file, Items[i]->_y);
    }
}

/* ------------------------------------------------------------------------- *
 * static void readSave(ifstream &file)                                      *
 *   file: file stream to read from                                          *
 *                                                                           *
 * Reads in all the items from a saved game file.                            *
 * ------------------------------------------------------------------------- */
void Item::readSave(ifstream &file)
{
    clearItems();
    int numitems = readInt(file);
    
    for (int i=0; i<numitems; i++)
    {
        int id = readInt(file);
        int x = readInt(file);
        int y = readInt(file);

        if (id == SODA)           addItem(new Soda(x, y));
        else if (id == GUM)       addItem(new Gum(x, y));
        else if (id == CHOCOLATE) addItem(new Chocolate(x, y));
        else if (id == CANDY)     addItem(new Candy(x, y));
        else if (id == DONUT)     addItem(new Donut(x, y));
        else if (id == ICECREAM)  addItem(new Icecream(x, y));
        else if (id == FLASK)     addItem(new Flask(x, y));
        else if (id == GUN)       addItem(new Gun(x, y));
        else if (id == DROP)      addItem(new Drop(x, y));
        else if (id == REDKEY)    addItem(new Redkey(x, y));
        else if (id == BLUKEY)    addItem(new Blukey(x, y));
        else if (id == GRNKEY)    addItem(new Grnkey(x, y));
        else if (id == YELKEY)    addItem(new Yelkey(x, y));
    }
}

/* ------------------------------------------------------------------------- *
 * void createItem(int x, int y, int id)                                     *
 *   x,y: location of the item                                               *
 *   id : id/type of item                                                    *
 *                                                                           *
 * Spawns a new item of type 'id' at location 'x,y'                          *
 * ------------------------------------------------------------------------- */
void Item::createItem(int x, int y, int id)
{
    switch (id)
    {   case SODA: addItem(new Soda(x, y)); return;
        case GUM: addItem(new Gum(x, y)); return;
        case CHOCOLATE: addItem(new Chocolate(x, y)); return;
        case CANDY: addItem(new Candy(x, y)); return;
        case DONUT: addItem(new Donut(x, y)); return;
        case ICECREAM: addItem(new Icecream(x, y)); return;
        case FLASK: addItem(new Flask(x, y)); return;
        case GUN: addItem(new Gun(x, y)); return;
        case DROP: addItem(new Drop(x, y)); return;
        case REDKEY: addItem(new Redkey(x, y)); return;
        case BLUKEY: addItem(new Blukey(x, y)); return;
        case GRNKEY: addItem(new Grnkey(x, y)); return;
        case YELKEY: addItem(new Yelkey(x, y)); return;
    }
}

/* ------------------------------------------------------------------------- *
 * static string getName(int id)                                             *
 *   id: id/type of item                                                     *
 *                                                                           *
 * Returns a string description of an item by id/type.                       *
 * ------------------------------------------------------------------------- */
string Item::getName(int id)
{
    switch (id)
    {   case SODA: return "Shikadi Soda";
        case GUM: return "Three-Tooth Gum";
        case CHOCOLATE: return "Candy Bar";
        case CANDY: return "Jawbreaker";
        case DONUT: return "Doughnut";
        case ICECREAM: return "Ice Cream Cone";
        case FLASK: return "Lifewater Flask";
        case GUN: return "Blaster Gun";
        case DROP: return "Liftwater Drop";
        case REDKEY: return "Red Gem";
        case BLUKEY: return "Blue Gem";
        case GRNKEY: return "Green Gem";
        case YELKEY: return "Yellow Gem";
    }
    return "";
}

/* ------------------------------------------------------------------------- *
 * static int getIcon(int id)                                                *
 *   id: id/type of item                                                     *
 *                                                                           *
 * Returns a tile number as an icon for the specified item type. (TED)       *
 * ------------------------------------------------------------------------- */
int Item::getIcon(int id)
{
    switch (id)
    {   case SODA:   return 563;
        case GUM:    return 565;
        case CHOCOLATE: return 567;
        case CANDY:  return 559;
        case DONUT:  return 561;
        case ICECREAM: return 685;
        case FLASK:  return 687;
        case GUN:    return 703;
        case DROP:   return 675;
        case REDKEY: return 1301;
        case BLUKEY: return 1303;
        case GRNKEY: return 1305;
        case YELKEY: return 1307;
    }
    return 0;
}

/* ------------------------------------------------------------------------- *
 * Item(int x, int y, int id)                                                *
 *   x,y: location of item (in pixels)                                       *
 *   id : actual type of item                                                *
 *   sframe: first frame in animation                                        *
 *   frames: total frames in animation                                       *
 *   delay : frame delay in animation                                        *
 *   xsize : width of the frames                                             *
 *   ysize : height of the frames                                            *
 *                                                                           *
 * The default constructor.                                                  *
 * ------------------------------------------------------------------------- */
Item::Item(int x, int y, int id, int sframe, int frames, int delay,
           int xsize, int ysize)
:_x(x), _y(y), _id(id), _sframe(sframe), _frames(frames), _delay(delay),
 _xsize(xsize), _ysize(ysize)
{ }

/* ------------------------------------------------------------------------- *
 * void draw(float zoom, int vpx, int vpy)                                   *
 *   zoom: level of zoom (ie: 2 = half size, .5 = double size)               *
 *   vpx,vpy: viewport coordinates ( also used by TED)                       *
 *                                                                           *
 * Draws the item. drawSprite is only called if the item is on the screen.   *
 * ------------------------------------------------------------------------- */
void Item::draw(float zoom, int vpx, int vpy)
{
    int x1 = (int)(_x - (_xsize/zoom)/2);
    int y1 = (int)(_y - (_ysize/zoom)/2);
    int x2 = (int)(_x + (_xsize/zoom)/2);
    int y2 = (int)(_y + (_ysize/zoom)/2);

    if (onCamera((float)x1, (float)y1, (float)x2, (float)y2))
    {
        int x = (int)(_x/zoom - (int)getCameraOrigX() - (_xsize/zoom / 2));
        int y = (int)(_y/zoom - (int)getCameraOrigY() - (_ysize/zoom / 2));

        int frame = _sframe + (int)(anmCounter() / _delay) % _frames;
        if (zoom == 1) drawSprite(frame, x+vpx, y+vpy);
        else drawSprite(frame, x+vpx, y+vpy, zoom);
    }
}

/* ------------------------------------------------------------------------- *
 * bool collision(int x1, int y1, int x2, int y2)                            *
 *   x1,y1,x2,y2: collision rectangle                                        *
 *                                                                           *
 * Returns true if the given collision rectangle intersects the item.        *
 * ------------------------------------------------------------------------- */
bool Item::collision(int x1, int y1, int x2, int y2)
{
    int ix1 = _x - (_xsize / 2); int iy1 = _y - (_ysize / 2);
    int ix2 = _x + (_xsize / 2); int iy2 = _y + (_ysize / 2);

    if (ix1 >= x1 && ix1 <= x2 && iy1 >= y1 && iy1 <= y2) return true;
    if (ix2 >= x1 && ix2 <= x2 && iy1 >= y1 && iy1 <= y2) return true;
    if (ix1 >= x1 && ix1 <= x2 && iy2 >= y1 && iy2 <= y2) return true;
    if (ix2 >= x1 && ix2 <= x2 && iy2 >= y1 && iy2 <= y2) return true;
    if (x1 >= ix1 && x1 <= ix2 && y1 >= iy1 && y1 <= iy2) return true;
    if (x2 >= ix1 && x2 <= ix2 && y1 >= iy1 && y1 <= iy2) return true;
    if (x1 >= ix1 && x1 <= ix2 && y2 >= iy1 && y2 <= iy2) return true;
    if (x2 >= ix1 && x2 <= ix2 && y2 >= iy1 && y2 <= iy2) return true;
    return false;

    /* This routine doesnt detect collisions such as:
                        +--+
                     +--|--|--+
                     |  |  |  |
                     +--|--|--+
                        +--+                       */
}

void TEDItem::drawItems(float zoom, int vpx, int vpy)
{   for (int i=0; i<Items.size(); i++) Items[i]->draw(zoom, vpx, vpy); }

PointsBonus::PointsBonus(int x, int y, int id, int sframe,
    int frames, int delay, int xsize, int ysize, int points)
:Item(x, y, id, sframe, frames, delay, xsize, ysize), _points(points)
{ }
void PointsBonus::giveBonus()
{   setPlayerScore(getPlayerScore()+_points);
    if (getPlayerScore() >= getPlayerNextlifeat())
    {   setPlayerNextlifeat(getPlayerNextlifeat()*2);
        setPlayerLives(getPlayerLives()+1);
        Sprite::addSprite(_x,_y-_ysize/2,620,626,3,11,16,false,false,-1);
    }
    else
    {   doPointsSprite(); }
    Audio::playSound(13);
}

Soda::Soda(int x, int y)                                           /* SODA */
:PointsBonus(x, y, SODA, 563, 2, 4, 10, 16, 100) {}
void Soda::doPointsSprite()
{ Sprite::addSprite(_x,_y-_ysize/2,569,575,3,16,32,false,false,-1); }

Gum::Gum(int x, int y)                                              /* GUM */
:PointsBonus(x, y, GUM, 565, 2, 4, 16, 16, 200) {}
void Gum::doPointsSprite()
{ Sprite::addSprite(_x,_y-_ysize/2,577,583,3,16,32,false,false,-1); }

Chocolate::Chocolate(int x, int y)                            /* CHOCOLATE */
:PointsBonus(x, y, CHOCOLATE, 567, 2, 4, 16, 16, 500) {}
void Chocolate::doPointsSprite()
{ Sprite::addSprite(_x,_y-_ysize/2,584,590,3,16,32,false,false,-1); }

Candy::Candy(int x, int y)                                        /* CANDY */
:PointsBonus(x, y, CANDY, 559, 2, 4, 16, 15, 1000) {}
void Candy::doPointsSprite()
{ Sprite::addSprite(_x,_y-_ysize/2,595,601,3,15,32,false,false,-1); }

Donut::Donut(int x, int y)                                        /* DONUT */
:PointsBonus(x, y, DONUT, 561, 2, 4, 16, 15, 2000) {}
void Donut::doPointsSprite()
{ Sprite::addSprite(_x,_y-_ysize/2,602,608,3,15,32,false,false,-1); }

Icecream::Icecream(int x, int y)                               /* ICECREAM */
:PointsBonus(x, y, ICECREAM, 685, 2, 4, 16, 15, 5000) {}
void Icecream::doPointsSprite()
{ Sprite::addSprite(_x,_y-_ysize/2,613,619,3,10,32,false,false,-1); }

Flask::Flask(int x, int y)                                        /* FLASK */
:Item(x, y, FLASK, 687, 2, 4, 12, 16) {}
void Flask::giveBonus()
{   setPlayerLives(getPlayerLives()+1);
    Sprite::addSprite(_x,_y-_ysize/2,620,626,3,11,16,false,false,-1);
    Audio::playSound(14);
}

Gun::Gun(int x, int y)                                              /* GUN */
:Item(x, y, GUN, 703, 2, 4, 16, 12) {}
void Gun::giveBonus()
{   setPlayerAmmo(getPlayerAmmo()+5);
    Sprite::addSprite(_x,_y-_ysize/2,638,644,3,16,32,false,false,-1);
    Audio::playSound(15);
}

Drop::Drop(int x, int y)                                           /* DROP */
:Item(x, y, DROP, 675, 4, 2, 16, 16) {}
void Drop::giveBonus()
{   setPlayerDrops(getPlayerDrops()+1);
    if (getPlayerDrops() > 99)
    {   setPlayerDrops(0);
        setPlayerLives(getPlayerLives()+1);
        Sprite::addSprite(_x,_y-_ysize/2,631,637,3,12,16,false,false,-1);
    }
    else
    { Sprite::addSprite(_x,_y,679,681,4,16,16,false,false,-1); }
    Audio::playSound(14);
}

Redkey::Redkey(int x, int y)                                     /* REDKEY */
:Item(x, y, REDKEY, 1301, 2, 4, 16, 12) {}
void Redkey::giveBonus()
{   setPlayerKeyred();
    Sprite::addSprite(_x,_y-_ysize/2,1319,1325,3,18,32,false,false,-1);
    Audio::playSound(16);
}

Blukey::Blukey(int x, int y)                                     /* BLUKEY */
:Item(x, y, BLUKEY, 1303, 2, 4, 16, 12) {}
void Blukey::giveBonus()
{   setPlayerKeyblu();
    Sprite::addSprite(_x,_y-_ysize/2,1319,1325,3,18,32,false,false,-1);
    Audio::playSound(16);
}

Grnkey::Grnkey(int x, int y)                                     /* GRNKEY */
:Item(x, y, GRNKEY, 1305, 2, 4, 15, 12) {}
void Grnkey::giveBonus()
{   setPlayerKeygrn();
    Sprite::addSprite(_x,_y-_ysize/2,1319,1325,3,18,32,false,false,-1);
    Audio::playSound(16);
}

Yelkey::Yelkey(int x, int y)                                     /* YELKEY */
:Item(x, y, YELKEY, 1307, 2, 4, 16, 12) {}
void Yelkey::giveBonus()
{   setPlayerKeyyel();
    Sprite::addSprite(_x,_y-_ysize/2,1319,1325,3,18,32,false,false,-1);
    Audio::playSound(16);
}
