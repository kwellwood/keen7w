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
 * Initialize the lift system by loading lifts.dat                           *
 * ------------------------------------------------------------------------- */
void Lift::init()
{
    ifstream file("LIFTS.DAT");
    if (!file.is_open()) error("Can't open file LIFTS.DAT");
    file >> Liftstoptile;
    file >> Liftlefttile;
    file >> Liftrighttile;
    file >> Liftuptile;
    file >> Liftdowntile;    
    file.close();
}

/* ------------------------------------------------------------------------- *
 * void readLifts(ifstream &file)                                            *
 *   file: file stream to read from                                          *
 *                                                                           *
 * Reads lifts and locators from a map file.                                 *
 * ------------------------------------------------------------------------- */
void Lift::readLifts(ifstream &file)
{

    // --- read locators ---
    for (int track=1; track<=16; track++)       // loop through tracks
    {
        Locators[track-1].clear();
        int numlocators = readInt(file);
        for (int l=1; l<=numlocators; l++)      // read locators on each track
        {
            int x = readInt(file);
            int y = readInt(file);
            int delay = readInt(file);
            int locksw = readInt(file);
            Locators[track-1].push_back(Locator(x,y,delay,locksw));
        }
    }

    // --- read lifts ---
    Lifts.clear();
    int numlifts = readInt(file);
    for (int l=1; l<=numlifts; l++)
    {
        int track = readInt(file);
        int target = readInt(file);
        Lifts.push_back(Lift(track, target, l-1));
        // lift objects are identified by the order in which they were
        // loaded (base 0) since lifts are never deleted.
    }
}

/* ------------------------------------------------------------------------- *
 * void drawLifts()                                                          *
 *                                                                           *
 * Draws all the lifts.                                                      *
 * ------------------------------------------------------------------------- */
void Lift::drawLifts()
{   for (int l=0; l<Lifts.size(); l++) Lifts[l].draw(); }

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
    // loaded into Lifts (base 0) since lifts are never deleted.
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
 * Lift(int track, int target, int id)                                       *
 *   track : the track the lift follows (1-16)                               *
 *   target: the locator the lift begins at                                  *
 *   id    : a unique number to identify this lift object                    *
 *                                                                           *
 * Default constructor for a lift.                                           *
 * ------------------------------------------------------------------------- */
Lift::Lift(int track, int target, int id)
:_track(track), _target(target), _id(id), _delay(0), _xv(0), _yv(0)
{
    _x = (float)Locators[track-1][target-1].x;
    _y = (float)Locators[track-1][target-1].y;
}

float Lift::x() { return _x; }
float Lift::y() { return _y; }

/* ------------------------------------------------------------------------- *
 * void draw()                                                               *
 *                                                                           *
 * Draw the lift if its on the screen.                                       *
 * ------------------------------------------------------------------------- */
void Lift::draw()
{
    // -- if lift is on screen --
    int x1 = (int)(_x - (52 / 2));
    int y1 = (int)(_y - (16 / 2));
    int x2 = (int)(_x + (52 / 2));
    int y2 = (int)(_y + (16 / 2));
    
    if (onCamera(x1, y1, x2, y2))
    {
        int x = (int)(_x - getCameraOrigX()) - (52 / 2);
        int y = (int)(_y - getCameraOrigY()) - (16 / 2);
        
        int frame = Liftstoptile;
        if (_xv <= -1) frame = Liftlefttile;
        if (_xv >= 1) frame = Liftrighttile;
        if (_yv <= -1) frame = Liftuptile;
        if (_yv >= 1) frame = Liftdowntile;
        if (_xv == 0 && _yv == 0) frame = Liftstoptile;
        if (frame != Liftstoptile)
        {   frame += (int)(anmCounter() / LIFTFRAMEDELAY) % 2; }
        
        drawSprite(frame, x, y);
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
    if (_x == Locators[_track-1][_target-1].x && 
        _y == Locators[_track-1][_target-1].y)
    {   // - - - - - - - - - At a Target - - - - - - - - -
        // -- decrease wait time --
        if (_delay > 0) _delay--;
        
        // -- set new target for lift --
        if (_delay == 0)
        {   if (Locators[_track-1][_target-1].lockswitch > 0)
            {   if (getSwitch(Locators[_track-1][_target-1].lockswitch))
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
        {   _x = Locators[_track-1][_target-1].x;
            _y = Locators[_track-1][_target-1].y;
            if (Locators[_track-1][_target-1].delay > 0)
            {   _delay = Locators[_track-1][_target-1].delay; }
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
    if (abs(_x - Locators[_track-1][_target-1].x) <= 2 &&
        abs(_y - Locators[_track-1][_target-1].y) <= 2) return true;
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
    if (_target > Locators[_track-1].size()) _target = 1;

    // -- set lift velocity --
    double xdist = (double)Locators[_track-1][_target-1].x - (double)_x;
    double ydist = (double)Locators[_track-1][_target-1].y - (double)_y;
    double factor = LIFTSPEED / sqrt(pow(xdist,2) + pow(ydist,2));
    _xv = (float)(xdist * factor);
    _yv = (float)(ydist * factor);
    
    if (_xv < .001 && _xv > 0) _xv = 0;
    if (_xv > -.001 && _xv < 0) _xv = 0;
    if (_yv > -.001 && _yv < 0) _yv = 0;
    if (_yv > -.001 && _yv < 0) _yv = 0;
}

/* ------------------------------------------------------------------------- *
 * bool collide(float x, float y)                                            *
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