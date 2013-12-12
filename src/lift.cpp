#include <iostream>
#include <fstream>
#include <cmath>
#include <allegro.h>
using namespace std;
#include "engine.h"
#include "lift.h"

/* ------------------------------------------------------------------------- *
 * void init()                                                               *
 *                                                                           *
 * Initialize the lift system by loading lifts.cfg                           *
 * ------------------------------------------------------------------------- */
void Lift::init()
{
    ifstream file("lifts.cfg");
    if (!file.is_open()) error("Can't open file lifts.cfg");
    file >> Liftstoptile;
    file >> Liftlefttile;
    file >> Liftrighttile;
    file >> Liftuptile;
    file >> Liftdowntile;
    file.close();
}

/* ------------------------------------------------------------------------- *
 * void addLocator(int x, int y, int track                                   *
 *   x,y   : place to add locator                                            *
 *   track : track to add locator to                                         *
 *   delay : time for lift to delay at this locator                          *
 *   locksw: switch which locks this locator                                 *
 *                                                                           *
 * Adds a locator to the specified track.                                    *
 * ------------------------------------------------------------------------- */
void Lift::addLocator(int x, int y, int track, int delay, int locksw)
{   Locators[track].push_back(Locator(x, y, delay, locksw)); }

/* ------------------------------------------------------------------------- *
 * void delLocator(int id)                                                   *
 *   id: unique id of locator to delete from collection                      *
 *                                                                           *
 * Removes a locator from a track, given its id.                             *
 * ------------------------------------------------------------------------- */
void Lift::delLocator(int id)
{
    int track = -1, slot = -1;      // track and slot of locator to delete
    for (int t=0; t<16; t++)
    {   for (int l=0; l<Locators[t].size(); l++)
        {   if (Locators[t][l].id == id)
            {   track = t; slot = l; break; }   // found the locator
        }
        if (track >= 0) break;                  // found the locator
    }
    
    if (track < 0) return;          // locator with 'id' was not found
    
    // Delete the lift if there's one at this locator
    int lift = -1;
    for (int i=0; i<Lifts.size(); i++)
    {   if (Lifts[i]._track == track && Lifts[i]._target == slot)
        {   lift = Lifts[i]._id; }  // There's a lift here on the same track,
                                    // delete it
        // Decrement the targets of all lifts on the track past slot because
        // all following slots will decrease by 1 after the locator delete
        if (Lifts[i]._track == track && Lifts[i]._target > slot)
        {   Lifts[i]._target--; }
    }
    if (lift >= 0) delLift(lift);   // Delete it

    vector<Locator>::iterator iter = Locators[track].begin();
    while (slot > 0) { iter++; slot--; }
    Locators[track].erase(iter);
}

/* ------------------------------------------------------------------------- *
 * void createLift(int x, int y, int track)                                  *
 *   x,y  : location of the lift                                             *
 *   track: track the lift will follow (0-15)                                *
 *                                                                           *
 * Places a lift on a track attached to the nearest locator. Lifts must be   *
 * created only at the locators on their track.                              *
 * ------------------------------------------------------------------------- */
void Lift::createLift(int x, int y, int track)
{
    float dist = 0;
    int locator = -1;
    
    // Look for the closest locator
    for (int i=0; i<Locators[track].size(); i++)
    {   // First locator we found
        if (locator == -1)
        {   locator = i;
            dist = sqrt((float)(
                (Locators[track][i].x-x)*(Locators[track][i].x-x) +
                (Locators[track][i].y-y)*(Locators[track][i].y-y)));
        }
        else
        {   // This isnt the first locator, so is it closer?
            float dist2 = sqrt((float)(
                (Locators[track][i].x-x)*(Locators[track][i].x-x) +
                (Locators[track][i].y-y)*(Locators[track][i].y-y)));
            if (dist2 < dist)
            {   locator = i; dist = dist2; }
        }
    }
    
    // Dont duplicate a lift at a locator
    bool targetTaken = false;
    for (int i=0; i<Lifts.size(); i++)
    {   if (Lifts[i]._track == track && Lifts[i]._target == locator)
        {   targetTaken = true; }       // There's already a lift there
    }
    
    if (locator >= 0 && !targetTaken)
    {   Lifts.push_back(Lift(track, locator)); }
}

/* ------------------------------------------------------------------------- *
 * void delLift(int id)                                                      *
 *   id: unique id of the lift to delete                                     *
 *                                                                           *
 * Removes a lift from the collection based on its ID.                       *
 * ------------------------------------------------------------------------- */
void Lift::delLift(int id)
{
    int l = -1;
    for (int i=0; i<Lifts.size(); i++)
    {   if (Lifts[i]._id == id) { l = i; break; } }

    if (l < 0) return;

    vector<Lift>::iterator iter = Lifts.begin();
    while (l > 0) { iter++; l--; }
    Lifts.erase(iter);
}

/* ------------------------------------------------------------------------- *
 * void readLifts(ifstream &file)                                            *
 *   file: file stream to read from                                          *
 *                                                                           *
 * Reads lifts and locators from a map file.                                 *
 * ------------------------------------------------------------------------- */
void Lift::readLifts(ifstream &file)
{
    NextID = 0;
    
    // --- read locators ---
    for (int track=0; track<16; track++)        // loop through tracks
    {
        Locators[track].clear();
        int numlocators;
        if (MAP_FORMAT_IN == K7W) { numlocators = readInt(file); }
        else { numlocators = readqbInt(file); }
        
        for (int l=0; l<numlocators; l++)       // read locators on each track
        {   int x, y, delay, locksw;
            if (MAP_FORMAT_IN == K7W)
            {   x = readInt(file); y = readInt(file);
                delay = readInt(file); locksw = readInt(file); }
            else
            {   x = readqbInt(file); y = readqbInt(file);
                delay = readqbInt(file); locksw = readqbInt(file); }
            Locators[track].push_back(Locator(x, y, delay, locksw));
        }
    }

    // --- read lifts ---
    Lifts.clear();
    int numlifts;
    if (MAP_FORMAT_IN == K7W) { numlifts = readInt(file); }
    else { numlifts = readqbInt(file); }
    for (int l=0; l<numlifts; l++)
    {   int track, target;
        if (MAP_FORMAT_IN == K7W)
        {   track = readInt(file); target = readInt(file); }
        else
        {   track = readqbInt(file); target = readqbInt(file); }
        Lifts.push_back(Lift(track, target));
    }
}

/* ------------------------------------------------------------------------- *
 * void writeMap(ofstream &file)                                             *
 *   file: file stream to write to                                           *
 *                                                                           *
 * Writes the lifts and locators to an open map file.                        *
 * ------------------------------------------------------------------------- */
void Lift::writeMap(ofstream &file)
{
    // --- Write locators on each track ---
    for (int track=0; track<16; track++)
    {
        int temp = Locators[track].size(); file.write((char*)&temp, 4);
        for (int l=0; l<Locators[track].size(); l++)
        {   file.write((char*)&Locators[track][l].x, 4);
            file.write((char*)&Locators[track][l].y, 4);
            file.write((char*)&Locators[track][l].delay, 4);
            file.write((char*)&Locators[track][l].lockswitch, 4);
        }
    }
    
    // --- Write lifts ---
    int temp = Lifts.size(); file.write((char*)&temp, 4);
    for (int l=0; l<Lifts.size(); l++)
    {   file.write((char*)&Lifts[l]._track, 4);
        file.write((char*)&Lifts[l]._target, 4);
    }   
}

/* ------------------------------------------------------------------------- *
 * int atLocator(int x, int y)                                               *
 *   x,y: location in pixels to check                                        *
 *                                                                           *
 * Checks if the location is inside the virtual box of a locator defined by  *
 * LOCATORWIDTH.                                                             *
 * ------------------------------------------------------------------------- */
int Lift::atLocator(int x, int y)
{
    for (int t=0; t<16; t++)
    {   for (int l=0; l<Locators[t].size(); l++)
        {   if (Locators[t][l].collision(x, y)) return Locators[t][l].id; }
    }
    return -1;
}

/* ------------------------------------------------------------------------- *
 * void drawLifts()                                                          *
 *                                                                           *
 * Draws all the lifts.                                                      *
 * ------------------------------------------------------------------------- */
void Lift::drawLifts()
{   for (int l=0; l<Lifts.size(); l++) Lifts[l].draw(1, 0, 0); }

/* ------------------------------------------------------------------------- *
 * void tickLifts()                                                          *
 *                                                                           *
 * Handles all of the lifts for one game cycle.                              *
 * ------------------------------------------------------------------------- */
void Lift::tickLifts()
{   for (int l=0; l<Lifts.size(); l++) Lifts[l].tick(); }

/* ------------------------------------------------------------------------- *
 * void collideLifts(float x, float y)                                       *
 *   x,y: point to check                                                     *
 *                                                                           *
 * Checks the lifts in the collection and returns the id of the first lift   *
 * that collides with the point, or returns -1 if none collide.              *
 * ------------------------------------------------------------------------- */
int Lift::collideLifts(float x, float y)
{   for (int i=0; i<Lifts.size(); i++)
    {   if (Lifts[i].collide(x, y)) return Lifts[i]._id; }
    return -1;
}

/* ------------------------------------------------------------------------- *
 * Lift getLift(int id)                                                      *
 *   id: unique id number of the lift                                        *
 *                                                                           *
 * Returns a lift object by its id number.                                   *
 * ------------------------------------------------------------------------- */
Lift Lift::getLift(int id)
{   
    // lift objects are identified by the order in which they were
    // loaded into Lifts (base 0) since lifts are never deleted ingame.
    return Lifts[id];
}

/* ------------------------------------------------------------------------- *
 * void clearLifts()                                                         *
 *                                                                           *
 * Clears all lifts and locators from the collections.                       *
 * ------------------------------------------------------------------------- */
void Lift::clearLifts()
{   for (int l=0; l<16; l++) Locators[l].clear(); Lifts.clear(); }

/* ------------------------------------------------------------------------- *
 * void clearLocators()                                                      *
 *                                                                           *
 * Clears all locators from the 16 tracks.                                   *
 * ------------------------------------------------------------------------- */
void Lift::clearLocators()
{
    for (int i=0; i<16; i++)
        Locators[i].clear();
}

/* ------------------------------------------------------------------------- *
 * static void writeSave(ofstream &file)                                     *
 *   file: file stream to write to                                           *
 *                                                                           *
 * Writes out all the lifts for a saved game file.                           *
 * ------------------------------------------------------------------------- */
void Lift::writeSave(ofstream &file)
{
    writeInt(file, Lifts.size());
    
    for (int i=0; i<Lifts.size(); i++)
    {   file.write((char*)&Lifts[i], sizeof(Lift)); }
}

/* ------------------------------------------------------------------------- *
 * static void readSave(ifstream &file)                                      *
 *   file: file stream to read from                                          *
 *                                                                           *
 * Reads in all the lifts from a saved game file.                            *
 * ------------------------------------------------------------------------- */
void Lift::readSave(ifstream &file)
{
    Lifts.clear();
    int numlifts = readInt(file);

    for (int i=0; i<numlifts; i++)
    {   Lift l = Lift();
        file.read((char*)&l, sizeof(Lift));
        Lifts.push_back(l);
    }
}

/* ------------------------------------------------------------------------- *
 * Lift(int track, int target, int id)                                       *
 *   track : the track the lift follows (1-16)                               *
 *   target: the locator the lift begins at                                  *
 *   id    : a unique number to identify this lift object                    *
 *                                                                           *
 * Default constructor for a lift.                                           *
 * ------------------------------------------------------------------------- */
Lift::Lift(int track, int target)
:_track(track), _target(target), _delay(0), _xv(0), _yv(0)
{
    _x = (float)Locators[track][target].x;
    _y = (float)Locators[track][target].y;
    _id = NextID; NextID++;
}

float Lift::x() { return _x; }
float Lift::y() { return _y; }

/* ------------------------------------------------------------------------- *
 * void draw(float zoom, int vpx, vpy)                                       *
 *   zoom: level of zoom (ie: 2 = half size, .5 = double size)               *
 *   vpx,vpy: viewport coordinates ( also used by TED)                       *
 *                                                                           *
 * Draw the lift if its on the screen.                                       *
 * ------------------------------------------------------------------------- */
void Lift::draw(float zoom, int vpx, int vpy)
{
    // -- if lift is on screen --
    int x1 = (int)(_x - (52/zoom / 2));
    int y1 = (int)(_y - (16/zoom / 2));
    int x2 = (int)(_x + (52/zoom / 2));
    int y2 = (int)(_y + (16/zoom / 2));
    
    if (onCamera(x1, y1, x2, y2))
    {
        int x = (int)((_x/zoom - getCameraOrigX()) - (52/zoom / 2));
        int y = (int)((_y/zoom - getCameraOrigY()) - (16/zoom / 2));
        
        int frame = Liftstoptile;
        if (_xv <= -1) frame = Liftlefttile;
        if (_xv >= 1) frame = Liftrighttile;
        if (_yv <= -1) frame = Liftuptile;
        if (_yv >= 1) frame = Liftdowntile;
        if (_xv == 0 && _yv == 0) frame = Liftstoptile;
        if (frame != Liftstoptile)
        {   frame += (int)(anmCounter() / LIFTFRAMEDELAY) % 2; }
        
        if (zoom == 1) drawSprite(frame, x+vpx, y+vpy);
        else drawSprite(frame, x+vpx, y+vpy, zoom);
    }
}

/* ------------------------------------------------------------------------- *
 * void tick()                                                               *
 *                                                                           *
 * Keeps the lift moving toward its target and handles actions when it       *
 * reaches its next target locator.                                          *
 * ------------------------------------------------------------------------- */
void Lift::tick()
{
    if (_x == Locators[_track][_target].x &&
        _y == Locators[_track][_target].y)
    {   // - - - - - - - - - At a Target - - - - - - - - -
        // -- decrease wait time --
        if (_delay > 0) _delay--;
        
        // -- set new target for lift --
        if (_delay == 0)
        {   if (Locators[_track][_target].lockswitch > 0)
            {   if (getSwitch(Locators[_track][_target].lockswitch))
                {   setNewTarget(); }
            }
            else
            {   setNewTarget(); }
        }
    }
    else
    {   // - - - - - - - - Moving to Target - - - - - - - -
        // -- record orig. position --
        float oldx = _x, oldy = _y;
        
        // -- move along track --
        _x += _xv;
        _y += _yv;

        // -- set delay if arriving at target --
        if (nearTarget())
        {   _x = Locators[_track][_target].x;
            _y = Locators[_track][_target].y;
            if (Locators[_track][_target].delay > 0)
            {   _delay = Locators[_track][_target].delay; }
        }
        
        // -- move player if attached to lift --
        if (getPlayerOnlift() == _id)
        {   setPlayerLoc(getPlayerX()+(_x-oldx), getPlayerY()+(_y-oldy)); }
    }

    /*
    if platform is at target then
      if platform has delay then decrease delay
      if platform has no delay and target not locked then
        set next target for platform
        set new velocities
      --
    --
    if platform is not at target then
      move toward target
      if platform close to target then
        platform at target
        if target has delay then copy to platform
      --
    --
    */
}

/* ------------------------------------------------------------------------- *
 * bool nearTarget()                                                         *
 *                                                                           *
 * Returns true if the lift is within a specified distance from its target   *
 * locator.                                                                  *
 * ------------------------------------------------------------------------- */
bool Lift::nearTarget()
{
    if (abs(_x - Locators[_track][_target].x) <= 2 &&
        abs(_y - Locators[_track][_target].y) <= 2) return true;
    return false;
}

/* ------------------------------------------------------------------------- *
 * void setNewTarget()                                                       *
 *                                                                           *
 * Finds the next locator on the track and sets it as target for the lift.   *
 * ------------------------------------------------------------------------- */
void Lift::setNewTarget()
{
    // -- set next target --
    _target++;
    if (_target == Locators[_track].size()) _target = 0;

    // -- set lift velocity --
    double xdist = (double)Locators[_track][_target].x - (double)_x;
    double ydist = (double)Locators[_track][_target].y - (double)_y;
    double factor = LIFTSPEED / sqrt(xdist*xdist+ydist*ydist);
    _xv = (float)(xdist * factor);
    _yv = (float)(ydist * factor);
    
    if (_xv < .001 && _xv > 0) _xv = 0;
    if (_xv > -.001 && _xv < 0) _xv = 0;
    if (_yv > -.001 && _yv < 0) _yv = 0;
    if (_yv > -.001 && _yv < 0) _yv = 0;
}

/* ------------------------------------------------------------------------- *
 * bool collide(float x, float y)                                            *
 *   x,y: possible point of collision                                        *
 *                                                                           *
 * Returns true if the point is inside the lift bounding box.                *
 * ------------------------------------------------------------------------- */
bool Lift::collide(float x, float y)
{
    float lx1 = _x - (56 / 2), ly1 = _y - 1;
    float lx2 = _x + (56 / 2), ly2 = _y + (18 / 2);

    if (x >= lx1 && x <= lx2 && y >= ly1 && y <= ly2) return true;
    return false;
}

string Lift::toStr(int n)
{   char buffer[11]; return string(itoa(n, buffer, 10)); }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* ------------------------------------------------------------------------- *
 * void draw(int track, int id, float zoom, int vpx, int vpy)                *
 *   track: track the locator is on                                          *
 *   id   : id of the locator                                                *
 *   zoom : level of zoom of the camera                                      *
 *   vpx,vpy: viewport coordinates (used by TED)                             *
 *                                                                           *
 * Draws the locator.                                                        *
 * ------------------------------------------------------------------------- */
void Lift::Locator::draw(int track, int id, float zoom, int vpx, int vpy)
{
    int x1 = (int)(x - (LOCATORWIDTH/zoom)/2);
    int y1 = (int)(y - (LOCATORWIDTH/zoom)/2);
    int x2 = (int)(x + (LOCATORWIDTH/zoom)/2);
    int y2 = (int)(y + (LOCATORWIDTH/zoom)/2);

    if (onCamera((float)x1, (float)y1, (float)x2, (float)y2))
    {
        int x2 = (int)(x/zoom - (int)getCameraOrigX() - (LOCATORWIDTH/zoom/2));
        int y2 = (int)(y/zoom - (int)getCameraOrigY() - (LOCATORWIDTH/zoom/2));

        drawBox(x2+vpx, y2+vpy, (int)(x2+LOCATORWIDTH/zoom+vpx),
            (int)(y2+LOCATORWIDTH/zoom+vpy), 233, EMPTY);
        if (zoom <= 1)
        {
            drawText("T" + toStr(track),
                (int)(x2 + LOCATORWIDTH/zoom/2+vpx - 8),
                (int)(y2 + LOCATORWIDTH/zoom/2+vpy - 6), 233);
            drawText("L" + toStr(id),
                (int)(x2 + LOCATORWIDTH/zoom/2+vpx - 8),
                (int)(y2 + LOCATORWIDTH/zoom/2+vpy + 2), 233);
        }
    }
}

/* ------------------------------------------------------------------------- *
 * bool collision(int x1, int y1)                                            *
 *   x1,y1: point to check                                                   *
 *                                                                           *
 * Checks if a point is inside the bounding box of the locator.              *
 * ------------------------------------------------------------------------- */
bool Lift::Locator::collision(int x1, int y1)
{   if (x1 >= x - LOCATORWIDTH/2 && x1 <= x + LOCATORWIDTH/2 &&
        y1 >= y - LOCATORWIDTH/2 && y1 <= y + LOCATORWIDTH/2) return true;
    return false;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void TEDLift::drawLifts(float zoom, int vpx, int vpy)
{   for (int l=0; l<Lifts.size(); l++) Lifts[l].draw(zoom, vpx, vpy); }

void TEDLift::drawLocators(float zoom, int vpx, int vpy)
{   for (int t=0; t<16; t++)
    {   for (int l=0; l<Locators[t].size(); l++)
        {   if (l < Locators[t].size() - 1)
            {   // Draw line to next locator
                Locator loc1 = Locators[t][l];
                Locator loc2 = Locators[t][l+1];
                int x1 = (int)(loc1.x/zoom) - (int)getCameraOrigX() + vpx;
                int y1 = (int)(loc1.y/zoom) - (int)getCameraOrigY() + vpy;
                int x2 = (int)(loc2.x/zoom) - (int)getCameraOrigX() + vpx;
                int y2 = (int)(loc2.y/zoom) - (int)getCameraOrigY() + vpy;
                drawLine(x1, y1, x2, y2, 15);
            }
            else
            {   // Draw line to first locator
                Locator loc1 = Locators[t][l];
                Locator loc2 = Locators[t][0];
                int x1 = (int)(loc1.x/zoom) - (int)getCameraOrigX() + vpx;
                int y1 = (int)(loc1.y/zoom) - (int)getCameraOrigY() + vpy;
                int x2 = (int)(loc2.x/zoom) - (int)getCameraOrigX() + vpx;
                int y2 = (int)(loc2.y/zoom) - (int)getCameraOrigY() + vpy;
                drawLine(x1, y1, x2, y2, 15);
            }
        }

        for (int l=0; l<Locators[t].size(); l++)
        {   Locators[t][l].draw(t, l, zoom, vpx, vpy); }
    }
}

