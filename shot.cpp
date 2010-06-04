#include <allegro.h>
using namespace std;
#include "engine.h"
#include "sprite.h"
#include "shot.h"

/* ------------------------------------------------------------------------- *
 * static void addShot(Shot* shot)                                           *
 *   shot: new shot object                                                   *
 *                                                                           *
 * Adds a new shot to the collection.                                        *
 * ------------------------------------------------------------------------- */
void Shot::addShot(Shot* shot)
{   Shots.push_back(shot); }

/* ------------------------------------------------------------------------- *
 * static void killShot(int s)                                               *
 *   s: shot's position in the Shots vector                                  *
 *                                                                           *
 * Removes the shot from Shots collection by position in the vector.         *
 * ------------------------------------------------------------------------- */
void Shot::killShot(int s)
{
    vector<Shot*>::iterator iter = Shots.begin();
    while (s > 0) { iter++; s--; }
    Shots.erase(iter);
}

/* ------------------------------------------------------------------------- *
 * static void drawShots()                                                   *
 *                                                                           *
 * Draws all of the shots in the collection.                                 *
 * ------------------------------------------------------------------------- */
void Shot::drawShots()
{   for (int s=0; s<Shots.size(); s++) Shots[s]->draw(); }

/* ------------------------------------------------------------------------- *
 * static void tickShots()                                                   *
 *                                                                           *
 * Handles all of the shots for one game cycle.                              *
 * ------------------------------------------------------------------------- */
void Shot::tickShots()
{   for (int s=0; s<Shots.size(); s++)
    {   Shots[s]->tick();
        if (Shots[s]->_destroy) { killShot(s); s--; }
    }
}

/* ------------------------------------------------------------------------- *
 * static void collideShot()                                                 *
 *   x1,y1,x2,y2: collision rectangle to check                               *
 *   owner: owner of shot (ENEMYSHOT, PLAYERSHOT)                            *
 *                                                                           *
 * Checks if the collision rectangle intersects any of the shots and         *
 * returns the type of shot                                                  *
 * ------------------------------------------------------------------------- */
bool Shot::collideShot(int x1, int y1, int x2, int y2, int owner)
{   bool collide=false;
    for (int s=0; s<Shots.size(); s++)
    {   if (Shots[s]->_owner == owner)
        {   if (Shots[s]->collision(x1, y1, x2, y2))
            {   Shots[s]->explode(); killShot(s); s--; collide=true; }
        }
    }
    return collide;
}

/* ------------------------------------------------------------------------- *
 * static void clearShots()                                                  *
 *                                                                           *
 * Erases all shots in the collection.                                       *
 * ------------------------------------------------------------------------- */
void Shot::clearShots()
{   for (int s=0; s<Shots.size(); s++) delete Shots[s]; Shots.clear(); }

/* ------------------------------------------------------------------------- *
 * Shot(...)                                                                 *
 *   x,y: location of shot                                                   *
 *   dir: direction (UP,DOWN,LEFT,RIGHT)                                     *
 *   sframe: first frame of animated shot                                    *
 *   eframe: last frame of animated shot                                     *
 *   delay : animation delay                                                 *
 *   xsize,ysize: dimensions of shot frames                                  *
 *   velocity: shot velocity                                                 *
 *                                                                           *
 * Default Shot constructor.                                                 *
 * ------------------------------------------------------------------------- */
Shot::Shot(float x, float y, int dir, int sframe, int eframe, int delay,
    int xsize, int ysize, float velocity, int owner)
:_x(x), _y(y), _frame(sframe), _sframe(sframe), _eframe(eframe), _delay(delay),
 _xsize(xsize), _ysize(ysize), _destroy(false), _owner(owner)
{
    if (dir == UP) { _xv = 0; _yv = -velocity; }
    else if (dir == DOWN) { _xv = 0; _yv = velocity; }
    else if (dir == LEFT) { _xv = -velocity; _yv = 0; }
    else if (dir == RIGHT) { _xv = velocity; _yv = 0; }
}

/* ------------------------------------------------------------------------- *
 * void draw()                                                               *
 *                                                                           *
 * Draws the shot if it is on the screen.                                    *
 * ------------------------------------------------------------------------- */
void Shot::draw()
{
    float x1 = _x - (_xsize / 2);
    float x2 = _x + (_xsize / 2);
    float y1 = _y - (_ysize / 2);
    float y2 = _y + (_ysize / 2);
    
    if (onCamera(x1, y1, x2, y2))
    {   int frame = _sframe + ((int)anmCounter()/_delay) % (_eframe-_frame);
        int sx = (int)(_x - getCameraOrigX() - (_xsize / 2));
        int sy = (int)(_y - getCameraOrigY() - (_ysize / 2));
        drawSprite(frame, sx, sy);
    }
}

/* ------------------------------------------------------------------------- *
 * void tick()                                                               *
 *                                                                           *
 * Handles the movement of the shot and map collisions for one game cycle.   *
 * ------------------------------------------------------------------------- */
void Shot::tick()
{
    // -- move shot --
    _x += _xv;
    _y += _yv;
    
    // -- map boundaries --
    if (_x < 0 || _x > mapWidth() * ts) _destroy = true;
    if (_y < 0 || _y > mapHeight() * ts) _destroy = true;
    
    // -- hit a wall --
    clipToMap();
    
    // -- gone out of range --
    if (!inRange(400)) _destroy = true;
}

/* ------------------------------------------------------------------------- *
 * void clipToMap()                                                          *
 *                                                                           *
 * Checks for collision between the shot and the map tiles. Calls explode()  *
 * and sets _destroy=true if collision detected.                             *
 * ------------------------------------------------------------------------- */
void Shot::clipToMap()
{
    int mx = (int)(_x / ts) + 1;
    int my = (int)(_y / ts) + 1;

    //hit a wall
    //hit a wall in the foreground (not over a pole)
    //check for slope hit
    //check for slope hit in foreground

    bool collide=false;
    if (tileType(getMap(mx, my)) == WALL) collide = true;
    if (tileType(getMapFG(mx, my)) == FGWALL &&
        tileType(getMap(mx, my)) != POLE) collide = true;
    if ((tileType(getMap(mx, my)) == ONEWAYFLOOR ||
         tileType(getMapFG(mx,my)) == ONEWAYFLOOR) &&
        tileType(getMap(mx, my)) != POLE && _yv > 0) collide = true;
    if (tileType(getMap(mx, my)) == SLOPE)
        collide = (getSlopeHeight((int)_x, (int)_y, 0) > 0);
    if (tileType(getMapFG(mx, my)) == SLOPE)
        collide = (getSlopeHeight((int)_x, (int)_y, 1) > 0);
    
    if (collide) { explode(); _destroy = true; }
}

/* ------------------------------------------------------------------------- *
 * bool collision(int x1, int y1, int x2, int y2)                            *
 *   x1,y1,x2,y2: collision rectangle to check                               *
 *                                                                           *
 * Returns true if the collision rectangle intersects the shot.              *
 * ------------------------------------------------------------------------- */
bool Shot::collision(int x1, int y1, int x2, int y2)
{
    int sx1 = (int)(_x-_xsize/2); int sy1 = (int)(_y-_ysize/2);
    int sx2 = (int)(_x+_xsize/2); int sy2 = (int)(_y+_ysize/2);

    if (sx1 >= x1 && sx1 <= x2 && sy1 >= y1 && sy1 <= y2) return true;
    if (sx2 >= x1 && sx2 <= x2 && sy1 >= y1 && sy1 <= y2) return true;
    if (sx1 >= x1 && sx1 <= x2 && sy2 >= y1 && sy2 <= y2) return true;
    if (sx2 >= x1 && sx2 <= x2 && sy2 >= y1 && sy2 <= y2) return true;
    if (x1 >= sx1 && x1 <= sx2 && y1 >= sy1 && y1 <= sy2) return true;
    if (x2 >= sx1 && x2 <= sx2 && y1 >= sy1 && y1 <= sy2) return true;
    if (x1 >= sx1 && x1 <= sx2 && y2 >= sy1 && y2 <= sy2) return true;
    if (x2 >= sx1 && x2 <= sx2 && y2 >= sy1 && y2 <= sy2) return true;
    return false;
}

/* ------------------------------------------------------------------------- *
 * void explode()                                                            *
 *                                                                           *
 * Creates the sprite explosion when a shot collides with something.         *
 * ------------------------------------------------------------------------- */
void Shot::explode() {}

/* ------------------------------------------------------------------------- *
 * bool inRange(int dist)                                                    *
 *                                                                           *
 * Returns true if this shot is within 'dist' pixels of player.              *
 * ------------------------------------------------------------------------- */
bool Shot::inRange(int dist)
{
    if (abs((int)(getPlayerX() - _x)) <= dist &&
        abs((int)(getPlayerY() - _y)) <= dist)
    {   return true; }
    return false;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Blaster::Blaster(float x, float y, int dir)
:Shot(x, y, dir, 705, 708, 1, 16, 16, 6, PLAYERSHOT)
{   _expsframe = 709; _expeframe = 710; _expdelay = 2;
    _expxsize = 16; _expysize = 16;
}
void Blaster::explode()
{   Sprite::addSprite(_x, _y, _expsframe, _expeframe, _expdelay, _expxsize,
        _expysize, false, false, -1);
    //if (inRange(240)) PlaySound(6);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

ArrowUp::ArrowUp(float x, float y)
:Shot(x, y, UP, 627, 628, 1, 8, 16, 10, ENEMYSHOT) {}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

ArrowDown::ArrowDown(float x, float y)
:Shot(x, y, DOWN, 629, 630, 1, 8, 16, 10, ENEMYSHOT) {}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

ArrowLeft::ArrowLeft(float x, float y)
:Shot(x, y, LEFT, 645, 646, 1, 16, 8, 10, ENEMYSHOT) {}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

ArrowRight::ArrowRight(float x, float y)
:Shot(x, y, RIGHT, 647, 648, 1, 16, 8, 10, ENEMYSHOT) {}