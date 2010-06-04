#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <allegro.h>
using namespace std;
#include "engine.h"
#include "player.h"
#include "shot.h"
#include "sprite.h"
#include "item.h"
#include "portal.h"
#include "lift.h"
#include "enemy.h"

/* ------------------------------------------------------------------------- *
 * Player()                                                                  *
 *                                                                           *
 * Constructor- initializes the data members and loads PLAYER.DAT.           *
 * ------------------------------------------------------------------------- */
Player::Player()         // Initialize all data members
:_x(0),_y(0),_mapx(0),_mapy(0),_xv(0),_yv(0),_xmax(0),_ymax(0),_xdir(0),
 _mapxdir(0),_sframe(0),_eframe(0),_frame(0),_delay(0),_xsize(0),_ysize(0),
 _txsize(0),_tysize(0),_onground(false),_onslope(false),_onlift(-1),
 _onball(-1),_adelay(0),_canjump(0),_jumpheight(0),_score(0),_nextlifeat(0),
 _lives(0),_ammo(0),_drops(0),_keyred(false),_keyblu(false),_keygrn(false),
 _keyyel(false),_scuba(false),_reload(0),_shooting(0), _aim(0),_action(0),
 _actiontime(0),_changeaction(0),_idletime(0), _frozen(false),
 _invincible(false),_usingportal(-1),_holdingjump(false),_stoppedjump(false),
 _dead(false),_ancientsrescued(0)
{
    ifstream file("PLAYER.DAT");
    if (!file.is_open())
    {   error("Can't open file PLAYER.DAT"); }
    file >> _adelay;                            // animation speed
    for (int f=0; f<=MAXPLAYERFRAMES; f++)
    {
        file >> _spriteFrame[f].tile;           // frame's tile number
        file >> _spriteFrame[f].width;          // frame's width
        file >> _spriteFrame[f].height;         // frame's height
    }
    file.close();

    _lives = 3;
    _ammo = 5;
    _score = 0;
}

/* ------------------------------------------------------------------------- *
 * ~Player()                                                                 *
 *                                                                           *
 * The destructor.                                                           *
 * ------------------------------------------------------------------------- */
Player::~Player()
{ }

/* ------------------------------------------------------------------------- *
 * void initState()                                                          *
 *                                                                           *
 * Resets the player to a starting state after use.                          *
 * ------------------------------------------------------------------------- */
void Player::initState()
{
    _action = MOVING;
    _actiontime = 0;
    _changeaction = 0;
    _idletime = 0;
    _xv = 0; _yv = 0;
    _frozen = false;
    _reload = 0;
    _holdingjump = false;
    _stoppedjump = false;
    _onlift = -1;
    _onball = -1;

    updateClipPoints();
}

/* ------------------------------------------------------------------------- *
 * void resetForLevel()                                                      *
 *                                                                           *
 * Called after loading a level, resets player movement variables etc.       *
 * ------------------------------------------------------------------------- */
void Player::resetForLevel()
{
    _dead = false;
    _onground = false; _onslope = false;
    _onlift = -1; _onball = -1;
    _canjump = 0;
    _reload = 0; _shooting = 0;
    _aim = -1;
    _action = MOVING; _actiontime = 0;
    _frozen = false;
    _usingportal = -1;
    _keyred = false; _keygrn = false; _keyblu = false; _keyyel = false;
}

/* ------------------------------------------------------------------------- *
 * void draw()                                                               *
 *                                                                           *
 * Draws the player's current frame.                                         *
 * ------------------------------------------------------------------------- */
void Player::draw()
{
    int px = (int)(_x - getCameraOrigX()) - (_txsize / 2);
    int py = (int)(_y - getCameraOrigY()) - (_tysize / 2);

    /*
    cout << setprecision(10)
         << _x << " - " << getCameraOrigX() << " = "
         << (double)_x - (double)getCameraOrigX() << " = (int)"
         << (int)((double)_x - (double)getCameraOrigX())
         << endl;
    */
    
    drawSprite(_frame, px, py);
}

/* ------------------------------------------------------------------------- *
 * bool collision(int x1, int y1, int x2, int y2)                            *
 *   x1,y1,x2,y2: collision rectangle to check                               *
 *                                                                           *
 * Returns true if the player's bounding box intersects the given rectangle. *
 * ------------------------------------------------------------------------- */
bool Player::collision(int x1, int y1, int x2, int y2)
{
    int px1 = (int)_clippoint[3].x; int py1 = (int)_clippoint[3].y;
    int px2 = (int)_clippoint[10].x; int py2 = (int)_clippoint[10].y;

    if (px1 >= x1 && px1 <= x2 && py1 >= y1 && py1 <= y2) return true;
    if (px2 >= x1 && px2 <= x2 && py1 >= y1 && py1 <= y2) return true;
    if (px1 >= x1 && px1 <= x2 && py2 >= y1 && py2 <= y2) return true;
    if (px2 >= x1 && px2 <= x2 && py2 >= y1 && py2 <= y2) return true;
    if (x1 >= px1 && x1 <= px2 && y1 >= py1 && y1 <= py2) return true;
    if (x2 >= px1 && x2 <= px2 && y1 >= py1 && y1 <= py2) return true;
    if (x1 >= px1 && x1 <= px2 && y2 >= py1 && y2 <= py2) return true;
    if (x2 >= px1 && x2 <= px2 && y2 >= py1 && y2 <= py2) return true;
    return false;
}

/* ------------------------------------------------------------------------- *
 * void move(int d)                                                          *
 *                                                                           *
 * Moves the player UP, DOWN, LEFT and RIGHT in the map.                     *
 * ------------------------------------------------------------------------- */
void Player::move(int d)
{
    _idletime = 0;
    
    switch (d)
    {
        // --- jump up, climb up ---
        case UP:
        // - Conditions to move up: -
        // 1. must not be frozen
        // 2. must be already jumping up OR on the ground OR climbing
        // 3. must not yet be at max jump height AND not let go of the button
        // 4. must be climbing with changeaction=0 OR not climbing
        // 5. must be not on the ground OR not holding the jump button
        if (moveMode() == JUMPMODE)
        {   if (!_frozen &&
                ((_yv < 0 && !_onground && _onball < 0 && _onlift < 0) ||
                 (_yv == 0 && _onground) || (_action == CLIMBING)) &&
                (_canjump > 0 && !_stoppedjump) &&
                ((_action == CLIMBING && _changeaction == 0) ||
                 (_action != CLIMBING)) &&
                (!_onground || !_holdingjump))
            {
                if (!_holdingjump) {/*PlaySound(9);*/}
                _holdingjump = true;
                _yv = -_ymax;
                if (_action == CLIMBING)        // -- jumping from a pole --
                {   _changeaction = maxfps/3;   // waittime to grab pole again
                    _yv /= 4;                   // 1/4 jumping power on pole
                }
                if (_action != POGO) _action = MOVING;
            }
        }
        else if (moveMode() == MAPMODE)
        {   _yv -= 2; }
        break;
        // --- climb down ---
        case DOWN:
        if (moveMode() == JUMPMODE)
        {   if (_action == HANGING && _changeaction == 0) _action == MOVING; }
        else if (moveMode() == MAPMODE)
        {   _yv += 2; }
        break;
        // --- walk left ---
        case LEFT:
        if (moveMode() == JUMPMODE)
        {
            if (_action == CLIMBING)
            {   _xdir = LEFT; }
            else if (_action  == HANGING)
            {   if (_changeaction == 0)
                {   if (_xdir == LEFT) climbUpLedge();
                    else _action = MOVING;
                }
            }
            else
            {   if (!_frozen)
                {   _xdir = LEFT;
                    if (_onground)            { _xv -= 2;  }
                    else if (_action == POGO) { _xv -= .3; }
                    else                      { _xv -= 1;  }   // .5
                }
            }
        }
        else if (moveMode() == MAPMODE)
        {   _xv -= 2; }
        break;
        // --- walk right ---
        case RIGHT:
        if (moveMode() == JUMPMODE)
        {
            if (_action == CLIMBING)
            {   _xdir = RIGHT; }
            else if (_action == HANGING)
            {   if (_changeaction == 0)
                {   if (_xdir == RIGHT) climbUpLedge();
                    else _action = MOVING;
                }
            }
            else
            {   if (!_frozen)
                {   _xdir = RIGHT;
                    if (_onground)            { _xv += 2;  }
                    else if (_action == POGO) { _xv += .3; }
                    else                      { _xv += 1;  }   // .5
                }
            }
        }
        else if (moveMode() == MAPMODE)
        {   _xv += 2; }
        break;
    }
}

/* ------------------------------------------------------------------------- *
 * void togglePogo()                                                         *
 *                                                                           *
 * Turns the pogostick on or off; activated by pogo key.                     *
 * ------------------------------------------------------------------------- */
void Player::togglePogo()
{
    _idletime = 0;
    
    if (_changeaction > 0) return;      // actions recently changed
    if (_frozen) return;                // player is frozen

    if (_action != POGO)
    {   _action = POGO; _actiontime = 0; _changeaction = maxfps / 5; }
    else
    {   _action = MOVING; _actiontime = 0; _changeaction = maxfps / 5; }
}

/* ------------------------------------------------------------------------- *
 * void shoot()                                                              *
 *                                                                           *
 * Shoots the player's blaster if he has ammo and its reloaded.              *
 * ------------------------------------------------------------------------- */
void Player::shoot()
{
    notIdle();
    
    if (_frozen || _reload != 0) return;

    float x, y; int dir;
    if (_aim == -1)
    {   if (_xdir == LEFT) { x = _x - _txsize / 2; dir = LEFT; }
        else /*xdir==RIGHT*/ { x = _x + _txsize / 2; dir = RIGHT; }
        y = _y;
    }
    else
    {   if (_aim == UP) { y = _y - _tysize / 2; dir = UP; }
        else /*aim==DOWN*/ { y = _y + _tysize / 2; dir = DOWN; }
        x = _x;
    }

    if (_ammo > 0)
    {   //PlaySound(4);
        _ammo--; Shot::addShot(new Blaster(x, y, dir));
    }
    else
    {   /*PlaySound(5);*/ }

    _reload = maxfps / 4;
    _shooting = maxfps / 5;
    if (_action == POGO) _action = MOVING;
    if (_onground && _action != CLIMBING) _frozen = true;        
}

/* ------------------------------------------------------------------------- *
 * void tryFlipSwitch()                                                      *
 *                                                                           *
 * Called when the up button is pressed, checks for a switch at player       *
 * location and flips it if there is one.                                    *
 * ------------------------------------------------------------------------- */
void Player::tryFlipSwitch()
{
    // -- must be standing on ground and not already flipping switch --
    if (!_onground || (_action == SWITCHING && _actiontime > 0)) return;
    
    int mx = (int)(_x / ts) + 1;
    int my = (int)((_y - (_tysize / 4)) / ts) + 1;
    
    // -- must be a switch here --
    if (tileType(getMap(mx, my)) != ASWITCH) return;
    
    // get number of switch here
    int s = getSwitchTag(mx, my);
    int s2 = getSwitchTag(mx, my-1);
    
    // no switch value here?
    if (s == 0) return;
    
    // --- flip the switch ---
    setSwitch(s, !getSwitch(s));
    
    // --- if bridge attached to switch ---
    if (s2 > 0) mapToggleBridge(s2);

    _action = SWITCHING;
    _actiontime = maxfps / 3;
    _xv = 0; _frozen = true;
    //PlaySound(5);
}

/* ------------------------------------------------------------------------- *
 * void tryTeleport()                                                        *
 *                                                                           *
 * Called when the up button is pressed; checks for a portal at player's     *
 * location and teleports into it if true.                                   *
 * ------------------------------------------------------------------------- */
void Player::tryTeleport()
{
    // -- must be standing on the ground --
    if (!_onground) return;
    
    // -- must not be teleporting --
    if (_action == TELEPORTING && _actiontime > 0) return;
    
    // -- check for a portal --
    int p = Portal::atPortal(_x, _y);
    if (p > -1)
    {   // A portal was found here
        Portal portal = Portal::getPortal(p);
        _x = (float)portal.x();
        _usingportal = p;
        _action = TELEPORTING;
        _actiontime = (int)(maxfps * .7);     // time=(actiontime/maxfps) sec
        _xv = 0; _yv = 0;
        _frozen = true;
        updateClipPoints();
    }
}

/* ------------------------------------------------------------------------- *
 * void tryGrabPole()                                                        *
 *                                                                           *
 * Called when the up button is pressed; checks for a pole at player         *
 * location and grabs it (starting climbing) if true.                        *
 * ------------------------------------------------------------------------- */
void Player::tryGrabPole()
{
    // -- too soon to grab a pole --
    if (_changeaction > 0) return;
    
    // -- get tile location of player --
    int x = (int)(_x /ts) + 1;
    int y = (int)((_y - (_ysize / 4)) / ts) + 1;

    // -- no pole at this location? --
    if (tileType(getMap(x, y)) != POLE && tileType(getMapFG(x, y)) != POLE)
    {   return; }

    // -- set climbing state --
    _xv = 0; _yv = 0;
    _action = CLIMBING; _actiontime = -1;
    _changeaction = (int)(maxfps / 3);
    _onground = false; _onslope = false;
    _x = (x * ts) - (ts / 2);
    _canjump = (int)(ts * (_jumpheight / 8));       // allow player to jump
                                                    // off pole at 1/8 power
}

/* ------------------------------------------------------------------------- *
 * void climbUpPole()                                                        *
 *                                                                           *
 * Continues player climbing up a pole.                                      *
 * ------------------------------------------------------------------------- */
void Player::climbUpPole()
{
    // -- tile location above player --
    int x = (int)(_x / ts) + 1;
    int y = (int)(((_y - 6) - (_tysize / 4)) / ts) + 1;
    
    // -- if at top of pole --
    // dont climb up, dont aim up
    if (tileType(getMap(x, y)) != POLE && tileType(getMapFG(x, y)) != POLE)
    {   _aim = -1; return; }
    
    // -- climb up pole --
    _y -= 2;
    updateClipPoints();
}

/* ------------------------------------------------------------------------- *
 * void climbDownPole()                                                      *
 *                                                                           *
 * Continues player climbing down a pole.                                    *
 * ------------------------------------------------------------------------- */
void Player::climbDownPole()
{
    // -- tile location above player --
    int x = (int)(_x / ts) + 1;
    int y = (int)(((_y + 8) - (_tysize / 4)) / ts) + 1;
    
    // -- if at bottom of pole, fall off --
    if (tileType(getMap(x, y)) != POLE && tileType(getMapFG(x, y)) != POLE)
    {   _action = MOVING; return; }
    
    // -- climb down pole --
    _y += 3.5;
    updateClipPoints();
}

/* ------------------------------------------------------------------------- *
 * void climbUpLedge()                                                       *
 *                                                                           *
 * Begins climbing up a ledge when player is hanging.                        *
 * ------------------------------------------------------------------------- */
void Player::climbUpLedge()
{
    if (_changeaction > 0) return;
    
    _action = HANGCLIMB;
    _actiontime = (int)(maxfps * .5);
}

/* ------------------------------------------------------------------------- *
 * void tryUnlockDoor()                                                      *
 *                                                                           *
 * Checks to see if the player has the key needed for the gem socket he's    *
 * standing on, and unlocks the door if so.                                  *
 * ------------------------------------------------------------------------- */
void Player::tryUnlockDoor()
{
    int mx = (int)(_x / ts) + 1;
    int my = (int)((_y + (_tysize / 4)) / ts) + 1;
    
    bool havekey=false;
    switch (slopeMask(getMapFG(mx, my)))
    {
        case 1: if (_keyred) { _keyred = false; havekey = true; } break;
        case 2: if (_keyblu) { _keyblu = false; havekey = true; } break;
        case 3: if (_keygrn) { _keygrn = false; havekey = true; } break;
        case 4: if (_keyyel) { _keyyel = false; havekey = true; } break;
    }

    if (havekey)
    {
        mapUnlockDoor(mx, my);

        _action = SWITCHING;
        _actiontime = maxfps / 3;
        _xv = 0; _frozen = true;
        //PlaySound(17);
    }
}

/* ------------------------------------------------------------------------- *
 * void tryEnterLevel()                                                      *
 *                                                                           *
 * Run when the player presses enter level key on the overhead map. Checks   *
 * if the player is standing on an ENTERLEVEL tiles and starts the level     *
 * if true.                                                                  *
 * ------------------------------------------------------------------------- */
void Player::tryEnterLevel()
{
    // The switchtag at the location of a tiletag2 ENTERLEVEL tile is
    // the level number to be loaded
    
    int tx = (int)(_x / ts) + 1;
    int ty = (int)(_y / ts) + 1;

    if (tileType2(getMap(tx, ty)) == ENTERLEVEL ||
        tileType2(getMapFG(tx, ty)) > 0)
    {   startLevel(getSwitchTag(tx, ty)); }
}

/* ------------------------------------------------------------------------- *
 * void kill()                                                               *
 *                                                                           *
 * Kills the player and begins death action.                                 *
 * ------------------------------------------------------------------------- */
void Player::kill()
{
    if (_action != DYING && !_invincible)
    {
        setGameMode(INGAME);
        _action = DYING;
        _actiontime = maxfps;
        _frozen = true;
        _onground = false;
        _xv = 0;
        _yv = -20;
        //Playsound(3);
    }
}

/* ------------------------------------------------------------------------- *
 * void tick()                                                               *
 *                                                                           *
 * Updates the player's state for one cycle of play.                         *
 * ------------------------------------------------------------------------- */
void Player::tick()
{
    if (moveMode() == JUMPMODE)
    {
        doPhysics();
        doOnLift();
        updateState();
        collideWithItems();
        collideWithSprites();
        collideWithShots();
        collideWithEnemies();
    
        _aim = -1;

        // -- idle time --
        if (_action == MOVING && _onground) _idletime++;
    }
    else if (moveMode() == MAPMODE)
    {
        mapModeDoPhysics();
        mapModeUpdateState();
    }
}

/* ------------------------------------------------------------------------- *
 * void readMap(ifstream &file)                                              *
 *   file: file stream to read from                                          *
 *                                                                           *
 * Reads the player data from a map file when a new level is loaded.         *
 * ------------------------------------------------------------------------- */
void Player::readMap(ifstream &file)
{
    // --- Initialize player state ---
    _x = readFloat(file);
    _y = readFloat(file);
    _xdir = readInt(file);
    initState();
}

/* ------------------------------------------------------------------------- *
 * void doPhysics()                                                          *
 *                                                                           *
 * Handles the player's physics, movement etc.                               *
 * ------------------------------------------------------------------------- */
void Player::doPhysics()
{
    // --- no physics if doing these action ---
    if (_action == CLIMBING) return;
    if (_action == HANGING || _action == HANGCLIMB) return;

    updateClipPoints();

    // - - - - - - - - - - - - MOVE PLAYER - - - - - - - - - - - -
    // --- Pogo stick ---
    if (_action == POGO) doPogoStick();

    // --- Gravity ---
    if (!_onground) _yv += Gravity;

    // --- Max speed ---
    if (_xv >  _xmax) _xv =  _xmax;
    if (_yv >  _ymax) _yv =  _ymax;
    if (_xv < -_xmax) _xv = -_xmax;
    if (_yv < -_ymax) _yv = -_ymax;

    // --- Move player ---
    _x += _xv;
    _y += _yv;

    // --- Friction ---
    if (_action != POGO) _xv *= Friction;
    if (_xv <  .2 && _xv > 0) _xv = 0;
    if (_xv > -.2 && _xv < 0) _xv = 0;

    // --- Map boundaries ---
    if (_x-_xsize/ 2 < 0) { _x=_xsize/2; _xv=0; }
    if (_y-_ysize/ 2 < 0) { _y=_ysize/2; _yv=0; }
    if (_x+_xsize/2 > (mapWidth()*ts)-1)
    {   _x=(mapWidth()*ts)-1-(_xsize/2); _xv=0; }
    if (_y+_ysize/2 > (mapHeight()*ts)-1)
    {   _y=(mapHeight()*ts)-1-(_ysize/2); _yv=0; }

    updateClipPoints();
    // - - - - - - - - - - - - - CLIPPING - - - - - - - - - - - - -

    /* If player is not on ground then start falling, or stick to
       slopes if standing on a slope                              */

    doOnGround();
    if (_onground) _stoppedjump = false;   // allow player to jump again
    updateClipPoints();
    clipToMap();
    updateClipPoints();

    // - - - - - - - - - CHECK NEW PLAYER STATE - - - - - - - - - -

    // --- Check jump height ---
    if (_onground) _canjump = (int)(ts * _jumpheight);
    // skip the the next line for jump cheat
    if (_yv < 0)        _canjump = (int)(_canjump + _yv);
    if (_canjump < 0)   _canjump = 0;

    doOnBall();                 // check if player is riding a bounder
}

/* ------------------------------------------------------------------------- *
 * void doPogoStick()                                                        *
 *                                                                           *
 * Bounces the player while on ground with a pogostick.                      *
 * ------------------------------------------------------------------------- */
void Player::doPogoStick()
{
    // --- if bouncing ---
    if (_onground)
    {
        //PlaySound 11
        _yv -= _ymax * 2;                        // bounce
        _canjump = (int)(ts * _jumpheight * 2);  // allow pogo to jump heigher
        _stoppedjump = false;                    // reset jump input vars
    }

    // --- stop input when starting to slow ascent ---
    if (_yv > -_ymax && !_holdingjump) _stoppedjump = true;
}

/* ------------------------------------------------------------------------- *
 * void doOnGround()                                                         *
 *                                                                           *
 * Checks what the player is standing on and updates the _onground and       *
 * _onslope variables accordingly.                                           *
 * ------------------------------------------------------------------------- */
void Player::doOnGround()
{
    // --- already standing on an object ---
    if (_onlift > -1 || _onball > -1)
    {   _onground = true; _onslope = false; return; }

    // --- previous ground values ---
    bool oldonslope = _onslope;
    bool oldonground = _onground;
    // --- map location of player's feet ---
    int mapx = (int)(_x / ts) + 1;
    int mapy = (int)((_y + (_ysize / 2)) / ts) + 1;

    // - - - - - - - - - - CHECK FOR GROUND TILE - - - - - - - - - -
    if (_yv >= 0)
    {
        //mapx=(int)(_x/ts)+1; mapy=(int)((_y+(_ysize/2))/ts)+1;
        if ((int)(_y+(_ysize/2))%ts >= ts-1)
        {   mapx = (int)(_x/ts)+1; mapy = (int)((_y+(_ysize/2))/ts)+2;
            if (tileType(getMap(mapx, mapy)) == WALL || 
                tileType(getMapFG(mapx, mapy)) == FGWALL ||
                tileType(getMapFG(mapx, mapy)) == ONEWAYFLOOR)
            {   _onground = true; _onslope = false;
                if ((int)(_y+(_ysize/2))%ts < ts-1) _y = (int)_y + 1;
            }
            else
            {   _onground = false; _onslope = false; }
        }
        else
        {   _onground = false; _onslope = false; }
    }
    else
    {   _onground = false; _onslope = false; }

    // - - - - - - - - - - - CHECK FOR SLOPES - - - - - - - - - - -
    if (tileType(getMap(mapx, mapy)) == SLOPE ||
        tileType(getMapFG(mapx, mapy)) == SLOPE)
    {
        int layer;
        float tmpx=_x, tmpy=_y+(_ysize/2);
        if (tileType(getMap(mapx, mapy)) == SLOPE) layer = 0; else layer = 1;
        int h1 = getSlopeHeight((int)tmpx, (int)tmpy, layer);
        int h2 = getSlopeHeight((int)tmpx, (int)tmpy+1, layer);
        if (h1 == 0 && h2 < 0)
        {   _onground = true; _onslope = true; }
        else
        {   _onground = false; _onslope = false; }
    }
    
    // - - - - - - - - - - KEEP PLAYER ON A SLOPE - - - - - - - - - -
    if (!_onground && oldonground && _yv >= 0)
    {   bool sticktoslope=false;
        mapx=(int)(_x/ts)+1; mapy=(int)((_y+(_ysize/2))/ts)+1;
        if (tileType(getMap(mapx, mapy)) == SLOPE ||
            tileType(getMapFG(mapx, mapy)) == SLOPE)
        {   sticktoslope=true; }
        else if (tileType(getMap(mapx, mapy+1)) == SLOPE ||
                 tileType(getMapFG(mapx, mapy+1)) == SLOPE)
        {   sticktoslope=true; mapy++;
            _y += (ts - ((int)(_y + (_ysize / 2) + ts) % ts));
        }
        else if (tileType(getMap(mapx, mapy+1)) == WALL ||
                 tileType(getMapFG(mapx, mapy+1)) == FGWALL)
        {   _y += (ts - (int)(_y + (_ysize / 2) + ts) % ts) - 1;
            _yv = 0; _onground = true;
        }

        if (sticktoslope)
        {   int layer;
            if (tileType(getMap(mapx, mapy)) == SLOPE) layer = 0;
            else layer = 1;
            _y += (ts - (int)(_y + (_ysize / 2) + 1) % ts) - 1;
            _y += getSlopeHeight((int)_x, (int)(_y+(_ysize/2)+1), layer);
            _yv = 0; _onground = true; _onslope = true;
        }
    }
}

/* ------------------------------------------------------------------------- *
 * void doOnLift()                                                           *
 *                                                                           *
 * Checks if the player is on a lift and handles movement.                   *
 * ------------------------------------------------------------------------- */
void Player::doOnLift()
{
    // --- is keen on a lift? ---
    _onlift = -1;
    if (_action == HANGCLIMB || _action == DYING) return;
    int l = Lift::collideLifts(_clippoint[2].x, _clippoint[2].y);
 
    if (l > -1 && _yv >= 0)
    {   // player is on lift l
        _onlift = l;
        _onball = -1;
        _yv = 0;
        _y = (Lift::getLift(l).y() - 1) - (_ysize / 2);
        _onground = true;
        _canjump = (int)(ts * _jumpheight);
        if (_action == HANGING) _action = MOVING;
        _holdingjump = false;
        _stoppedjump = false;
        updateClipPoints();
    } 
}

/* ------------------------------------------------------------------------- *
 * void doOnLift()                                                           *
 *                                                                           *
 * Checks if the player is on a Bounder and handles movement.                *
 * ------------------------------------------------------------------------- */
void Player::doOnBall()
{
    // --- is keen on a bounder? ---
    _onball = -1;
    if (_action == HANGCLIMB || _action == DYING || _onlift > -1) return;
    if (_yv < 0) return;

    int onball = Enemy::standingOnEnemy(_x, _y+_ysize/2);
    if (onball > -1)
    {   _onball = onball;
        
        _yv = 0;
        _y = Enemy::getEnemyByID(onball)->top() - (_ysize / 2) + 1;
        //player(0).y = (enemy(e).y - (enemy(e).tysize / 2)) - (player(0).ysize / 2) + 1!
        _onground = true;
        _canjump = (int)(ts * _jumpheight);
        if (_action == HANGING) _action = MOVING;
        _holdingjump = false;
        _stoppedjump = false;
        updateClipPoints();
    }
}

/* ------------------------------------------------------------------------- *
 * void clipToMap()                                                          *
 *                                                                           *
 * Performs clipping between player and the map. Calls other functions if    *
 * special tiles are touched such as gem sockets or end level tiles.         *
 * ------------------------------------------------------------------------- */
void Player::clipToMap()
{
    // --- No clipping while dying ---
    if (_action == DYING) return;

    for (int c=1; c<=8; c++)
    {   switch (mapHitPoint(_clippoint[c].x, _clippoint[c].y))
        {   case WALL:
            hitWall(c,_clippoint[c].x,_clippoint[c].y); break;
            case SLOPE:
            if(c==2) hitSlope(_clippoint[c].x, _clippoint[c].y, 0); break;
            case HAZZARD: kill(); break;
            case ONEWAYFLOOR:
            if(c == 2 && _yv >= 0)
            {   hitWall(c, _clippoint[c].x, _clippoint[c].y); } break;
            case GEMSOCKET:
            if(c == 2) tryUnlockDoor(); break;
            case FGSLOPE:
            if(c==2) hitSlope(_clippoint[c].x, _clippoint[c].y, 1); break;
            case FGWALL:
            hitWall(c, _clippoint[c].x, _clippoint[c].y); break;
            case ENDLEVEL: setLevelComplete(); break;
            case SCUBAGEAR:
            if (!_scuba)
            {   _scuba=true; showGotScubaGear(); setLevelComplete(); }
            break;
        }
        updateClipPoints();
    }
}

/* ------------------------------------------------------------------------- *
 * void hitWall(int p, float hx, float hy)                                   *
 *   p    : clippoint that hit the tile                                      *
 *   hx,hy: location of collision                                            *
 *                                                                           *
 * Performs clipping between player and a single wall tile. Uses the players *
 * hitpoint colliding with the tile to move him back outside of it again.    *
 * ------------------------------------------------------------------------- */
void Player::hitWall(int p, float hx, float hy)
{
    /* 3--1--4
       |     |
       5     6
       |     |
       7     8
       |     |
       9--2-10 */

    int side = -1;
    float nx, ny;
    
    if (p == 1)
    {   ny = (int)((hy - ((int)hy % ts)) + ts); side = UP; }
    else if (p == 2)
    {   ny = (int)((hy - ((int)hy % ts)) - 1); side = DOWN; }
    else if (p == 3 || p == 5 || p == 7 || p == 9)
    {   nx = ((int)((int)hx / ts) + 1) * ts; side = RIGHT; }
    else if (p == 4 || p == 6 || p == 8 || p == 10)
    {   nx = (int)(hx / ts) * ts - 1; side = LEFT; }

    switch (side)
    {
        case UP:
        _yv=0; _y=(int)(ny+(_ysize/2))+1; /*PlaySound 8*/ break;
        case DOWN: _yv=0; _y=(int)(ny-(_ysize/2)); _onground=true; break;
        case LEFT:
        if (p == 4 || p == 6 || p == 8 || p == 10) _x=(int)(nx-(_xsize/2));
        _xv=0; break;
        case RIGHT:
        if (p == 3 || p == 5 || p == 7 || p == 9) _x=(nx+(_xsize/2));
        _xv=0; break;
    }
    
    updateClipPoints();
    
    // --- check for hitting a ledge ---
    if ((side == LEFT || side == RIGHT) && (p == 5 || p == 6))
    {   if ((int)_clippoint[p].y % ts < ts / 2)
        {
            checkForLedge(side, (int)(_clippoint[p].x / ts + 1),
                          (int)(_clippoint[p].y / ts + 1));
        }
    }
}

/* ------------------------------------------------------------------------- *
 * void hitSlope(float x, float y, int l)                                    *
 *   x,y: location of clippoint collision                                    *
 *   l  : layer of collision (0=background, 1=foreground)                    *
 *                                                                           *
 * Performs clipping between player and a single slope tile. Moves the       *
 * player by his bottom (_clippoint[2]) up to stand on the slope.            *
 * ------------------------------------------------------------------------- */
void Player::hitSlope(float x, float y, int l)
{
    int h = getSlopeHeight((int)x, (int)y, l);
    
    // --- correct bad values ---
    if (h > ts) h = ts;
    if (h < -ts) h = -ts;

    if (h != 0)
    {
        _y = (int)(_y + h);
        _yv = 0;
        _onground = true;
        _onslope = true;
        //_holdingjump = false;
        _stoppedjump = false;
    }
}

/* ------------------------------------------------------------------------- *
 * void collideWithItems()                                                   *
 *                                                                           *
 * Checks if the player has collided with an item and removes it if true.    *
 * ------------------------------------------------------------------------- */
void Player::collideWithItems()
{
    int itm = Item::collideItem((int)_clippoint[3].x, (int)_clippoint[3].y,
        (int)_clippoint[10].x, (int)_clippoint[10].y);

    if (itm > -1) Item::killItem(itm);
}

/* ------------------------------------------------------------------------- *
 * void collideWithSprites()                                                 *
 *                                                                           *
 * Checks if the player has collided with any deadly sprites and kills him   *
 * if true.                                                                  *
 * ------------------------------------------------------------------------- */
void Player::collideWithSprites()
{
    if (Sprite::collideSprite((int)_clippoint[3].x, (int)_clippoint[3].y,
        (int)_clippoint[10].x, (int)_clippoint[10].y) > -1) kill();
}

/* ------------------------------------------------------------------------- *
 * void collideWithShots()                                                   *
 *                                                                           *
 * Checks if the player has collided with any enemy shots and kills him      *
 * if true.                                                                  *
 * ------------------------------------------------------------------------- */
void Player::collideWithShots()
{
    if (Shot::collideShot((int)_clippoint[3].x, (int)_clippoint[3].y,
        (int)_clippoint[10].x, (int)_clippoint[10].y, ENEMYSHOT)) kill();
}

/* ------------------------------------------------------------------------- *
 * void collideWithEnemies()                                                 *
 *                                                                           *
 * Checks if the player has collided with any deadly enemies and kills him   *
 * if true.                                                                  *
 * ------------------------------------------------------------------------- */
void Player::collideWithEnemies()
{
    if (_action == TELEPORTING) return;

    int touchingEnemyType = Enemy::collideEnemy((int)_clippoint[3].x,
        (int)_clippoint[3].y, (int)_clippoint[10].x, (int)_clippoint[10].y);

    if (touchingEnemyType == DEADLYENEMY) kill();
    else if (touchingEnemyType == ENDLEVELENEMY) doAncientRescued();
}

/* ------------------------------------------------------------------------- *
 * void checkForLedge(int side, int mx, int my)                              *
 *   side : side of the tile the player is on                                *
 *   mx,my: map location of player (in tiles)                                *
 *                                                                           *
 * Checks if the player can grab onto a ledge here. _action set to HANGING   *
 * if player does grab the ledge.                                            *
 * ------------------------------------------------------------------------- */
void Player::checkForLedge(int side, int mx, int my)
{
    /* +--+--+  . = mx, my
       |1 |  |
       |  |  |
       +--+--+
       |##|. |
       |##|  |
       +--+--+ */

    if (side == LEFT) mx++;             // left side of tile (not the player)
    else if (side == RIGHT) mx--;       // right side of the tile
    
    // --- check for wall in tile 1 area (see diagram) ---
    int fg = tileType(getMapFG(mx, my-1));
    int bg = tileType(getMap(mx, my-1));
    if (fg == SLOPE || fg == ONEWAYFLOOR || fg == FGWALL || bg != PASSIVE)
    {   return; }
    
    // --- cant be using pogo or shooting ---
    if (_action != MOVING) return;
    if (_yv <= 0) return;
    
    _y = (my * ts - ts - 4) + (_ysize / 2);
    updateClipPoints();
    
    _action = HANGING;
    _actiontime = -1;
    _changeaction = maxfps / 6;
    _frozen = true;
    _holdingjump = false;
    _stoppedjump = false;
    _canjump = 0;
    _yv = 0; _xv = 0;
}

/* ------------------------------------------------------------------------- *
 * void finishClimbingLedge()                                                *
 *                                                                           *
 * Transitions the player to moving again after climbing up a ledge.         *
 * ------------------------------------------------------------------------- */
void Player::finishClimbingLedge()
{
    int cp=0, mx=0, my=0;
    if (_xdir == LEFT)
    {   mx = (int)(_clippoint[5].x / ts + 1) - 1;
        my = (int)(_clippoint[5].y / ts + 1);
    }
    else if (_xdir = RIGHT)
    {   mx = (int)(_clippoint[6].x / ts + 1) + 1; 
        my = (int)(_clippoint[6].y / ts + 1);
    }

    _x = (mx * ts - ts) + (ts * .5);
    _y = ((my * ts - ts) - 1) - (_ysize / 2);
    _onground = true;
    
    updateClipPoints();
}

/* ------------------------------------------------------------------------- *
 * void teleport()                                                           *
 *                                                                           *
 * Finished the TELEPORTING action by moving the player to the destination   *
 * of the teleporter.                                                        *
 * ------------------------------------------------------------------------- */
void Player::teleport()
{
    Portal p = Portal::getPortal(_usingportal);
    _x = (float)p.destx();
    _y = (float)p.desty();
    _usingportal = -1;
    updateClipPoints();
    focusCamera();
}

/* ------------------------------------------------------------------------- *
 * void updateState()                                                        *
 *                                                                           *
 * Updates the player's current state, including animation frame.            *
 * ------------------------------------------------------------------------- */
void Player::updateState()
{
    // -- start idletime --
    if (_action == MOVING && _xv == 0 && _yv == 0 && !_frozen &&
        _idletime > maxfps * 10)
    {
        _action = IDLE;
        _actiontime = (int)(maxfps * 5.75);
        _frozen = true;
    }

    // -- continue idletime --
    if (_action == IDLE)
    {
        if (_idletime > 0)
        {   if (_actiontime == 18) _actiontime = 199; }
        else if (_idletime < maxfps && _actiontime > 17)
        {   _actiontime = 17; }
    }
    
    // -- actiontime --
    if (_actiontime > 0) _actiontime--;
    if (_actiontime < 0) _actiontime = 0;

    // -- changeaction --
    if (_changeaction > 0) _changeaction--;
    if (_changeaction < 0) _changeaction = 0;

    // -- shooting --
    if (_shooting > 0)
    {   if (_onground) { _xv = 0; _frozen = true; }
        _shooting--;
    }

    // -- reload --
    if (_reload > 0) { _reload--; }
    
    // -- unfreeze if moving --
    if (_action == MOVING && _shooting == 0) _frozen = false;

    // -- player direction --
    if (_action == MOVING || _action == POGO)
    {   if (_xv < 0) _xdir = LEFT;
        if (_xv > 0) _xdir = RIGHT;
    }

    // - - - - - - - - DYING - - - - - - - -
    if (_action == DYING)
    {
        if (_actiontime == 0)
        {   if (_lives > 0) { _dead = true; }
            else { _actiontime = 1; setGameOver(); }
        }
    }
    // - - - - - - - TELEPORT - - - - - - -
    else if (_action == TELEPORTING)
    {   if (_actiontime == 0) teleport(); }
    // - - - - - - - CLIMBING - - - - - - -
    else if (_action == CLIMBING)
    {   _holdingjump = false; _stoppedjump = false; }
    // - - - - - - HANG CLIMBING - - - - - -
    else if (_action == HANGCLIMB)
    {   if (_actiontime == 0) finishClimbingLedge(); }

    // -- resume walking --
    if (_actiontime == 0)
    {
        if (_action != CLIMBING && _action != POGO && _action != HANGING &&
            _action != DYING)
        {   _action = MOVING; }
    }

    // -- select animation frame --
    if (_shooting > 0)
    {   setFrameShooting(); }
    else
    {
        switch (_action)
        {
            case MOVING:      setFrameMoving();      break;
            case SWITCHING:   setFrameSwitching();   break;
            case DYING:       setFrameDying();       break;
            case TELEPORTING: setFrameTeleporting(); break;
            case CLIMBING:    setFrameClimbing();    break;
            case POGO:        setFramePogo();        break;
            case HANGING:     setFrameHanging();     break;
            case HANGCLIMB:   setFrameHangClimb();   break;
            case IDLE:        setFrameIdle();        break;
        }
    }
}

/* ------------------------------------------------------------------------- *
 * void setFrame*****()                                                      *
 *                                                                           *
 * This collection of methods sets the player's frame depending on the       *
 * current action. Each method modifies _frame, _txsize and _tysize.         *
 * ------------------------------------------------------------------------- */
void Player::setFrameShooting()
{
    int framedataslot;

    // --- Shooting while on a pole ---
    if (_action == CLIMBING)
    {
        if (_aim == -1)
        {   if (_xdir == LEFT)  framedataslot = 31;     // POLELEFT
            if (_xdir == RIGHT) framedataslot = 34; }   // POLERIGHT
        else if (_aim == UP)
        {   if (_xdir == LEFT)  framedataslot = 32;     // POLELEFTUP
            if (_xdir == RIGHT) framedataslot = 35; }   // POLERIGHTUP
        else if (_aim == DOWN)
        {   if (_xdir == LEFT)  framedataslot = 33;     // POLELEFTDOWN
            if (_xdir == RIGHT) framedataslot = 36; }   // POLERIGHTDOWN
    }
    // --- Shooting while on the ground --
    else if (_onground)
    {
        // - Not aiming up or down -
        if (_aim == -1)
        {   if (_xdir == LEFT)  framedataslot = 14;     // GROUNDLEFT
            if (_xdir == RIGHT) framedataslot = 15;     // GROUNDRIGHT
        }
        // - Aiming up -
        else framedataslot = 16;                         // GROUNDUP
    }
    // --- Shooting while in the air ---
    else
    {
        // - Not aiming up or down -
        if (_aim == -1)
        {   if (_xdir == LEFT)  framedataslot = 17;     // AIRLEFT
            if (_xdir == RIGHT) framedataslot = 18;     // AIRRIGHT
        }
        // - Aiming up -
        else if (_aim == UP) framedataslot = 19;        // AIRUP
        // - Aiming down -
        else if (_aim == DOWN) framedataslot = 20;      // AIRDOWN
    }
    
    _frame  = _spriteFrame[framedataslot].tile;
    _txsize = _spriteFrame[framedataslot].width;
    _tysize = _spriteFrame[framedataslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void Player::setFrameMoving()
{
    int framedataslot=-1, anmoffset=0;
    
    // --- standing on the ground ---
    if (_onground)
    {
        if (_xdir == LEFT)
        {   // Walking to the left
            if (_xv < 0)
            {   framedataslot = 1;
                anmoffset = (anmCounter() / _adelay) % _spriteFrame[2].tile; }
            // Standing facing left
            else { framedataslot = 0; }
        }
        else if (_xdir == RIGHT)
        {   // Walking to the right
            if (_xv > 0)
            {   framedataslot = 4;
                anmoffset = (anmCounter() / _adelay) % _spriteFrame[5].tile; }
            // Standing facing right
            else { framedataslot = 3; }
        }
    }
    // --- jumping/falling in the air ---
    else
    {
        if (_xdir == LEFT)
        {   if (_yv < -(_ymax / 4))     framedataslot = 6;     // jump left
            else if (_yv > (_ymax / 4)) framedataslot = 8;     // fall left
            else                        framedataslot = 7;     // apex left
        }
        else if (_xdir == RIGHT)
        {   if (_yv < -(_ymax / 4))     framedataslot = 9;     // jump right
            else if (_yv > (_ymax / 4)) framedataslot = 11;    // fall right
            else                        framedataslot = 10;    // apex right
        }
    }

    _frame  = _spriteFrame[framedataslot].tile + anmoffset;
    _txsize = _spriteFrame[framedataslot].width;
    _tysize = _spriteFrame[framedataslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void Player::setFrameSwitching()
{
    _frame = _spriteFrame[21].tile;
    _txsize = _spriteFrame[21].width;
    _tysize = _spriteFrame[21].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void Player::setFrameDying()
{
    _frame = _spriteFrame[22].tile;
    _txsize = _spriteFrame[22].width;
    _tysize = _spriteFrame[22].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void Player::setFrameTeleporting()
{
    _frame = _spriteFrame[23].tile + (int)(((maxfps * .7) - _actiontime) /
             ((maxfps * .7) / _spriteFrame[24].tile));

    _txsize = _spriteFrame[23].width;    
    _tysize = _spriteFrame[23].height + ((_frame-_spriteFrame[23].tile)*8);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void Player::setFrameClimbing()
{
    /* 25=climbing up left
       26=climbing up left frames
       27=climbing up right
       28=climbing up right frames
       29=climbing down
       30=climbing down frames */

    int framedataslot=0, anmoffset=0;
    switch(_aim)
    {   // -- climbing up --
        case UP:
        if (_xdir == LEFT)
        {   framedataslot = 25;
            anmoffset = (int)(anmCounter()/_adelay)%_spriteFrame[26].tile; }
        else if (_xdir == RIGHT)
        {   framedataslot = 27;
            anmoffset = (int)(anmCounter()/_adelay)%_spriteFrame[28].tile; }
        break;
        // -- sliding down --
        case DOWN:
        framedataslot = 29;
        anmoffset = (int)(anmCounter()/(_adelay*1.5))%_spriteFrame[30].tile;
        break;
        // -- hanging still --
        case -1:
        if (_xdir == LEFT) { framedataslot = 25; anmoffset = 0; }
        else if (_xdir == RIGHT) { framedataslot = 27; anmoffset = 0; }
        break;
    }

    _frame  = _spriteFrame[framedataslot].tile + anmoffset;
    _txsize = _spriteFrame[framedataslot].width;
    _tysize = _spriteFrame[framedataslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void Player::setFramePogo()
{
    int framedataslot=0;
    if (_xdir == LEFT)
    {   if (_yv < -_ymax / 2) framedataslot = 37; else framedataslot = 38; }
    else if (_xdir == RIGHT)
    {   if (_yv < -_ymax / 2) framedataslot = 39; else framedataslot = 40; }

    _frame = _spriteFrame[framedataslot].tile;
    _txsize = _spriteFrame[framedataslot].width;
    _tysize = _spriteFrame[framedataslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void Player::setFrameHanging()
{
    int framedataslot=0;
    if (_xdir == LEFT) framedataslot = 41;
    else if (_xdir == RIGHT) framedataslot = 42;

    _frame = _spriteFrame[framedataslot].tile;
    _txsize = _spriteFrame[framedataslot].width;
    _tysize = _spriteFrame[framedataslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void Player::setFrameHangClimb()
{   
    int framedataslot=0;                       // num frames in animation \/
    int anmoffset = (int)(((maxfps * .5) - _actiontime) / ((maxfps * .5) / 4));
    int xsize=0, ysize=0;
    
    if (_xdir == LEFT)
    {   framedataslot = 43;
        switch (anmoffset)
        {   case 0: xsize = 22; ysize = 76; break;
            case 1: xsize = 28; ysize = 92; break;
            case 2: xsize = 40; ysize = 94; break;
            case 3: xsize = 44; ysize = 96; break;
        }
    }
    else if (_xdir == RIGHT)
    {   framedataslot = 44;
        switch (anmoffset)
        {   case 0: xsize = 22; ysize = 76; break;  // xsize = 22
            case 1: xsize = 16; ysize = 92; break;  // xsize = 16
            case 2: xsize = 10;  ysize = 94; break; // xsize = 4
            case 3: xsize = 6;  ysize = 96; break;  // xsize = 0
        }
    }

    _frame = _spriteFrame[framedataslot].tile;
    _txsize = xsize;
    _tysize = ysize;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void Player::setFrameIdle()
{
    /* _actiontime
       ->maxfps*5.75
       -start 1
         [maxfps] - sitting down
       -end 1
       -start 2                     <-+
         [maxfps*4] - reading         |
       -end 2                         |
       -start 3                       |
         [maxfps/2] - turning page    |
       -end 3                      ---+
       -start 4
        [maxfps] - standing up
       ->end 4 (0)                       */

    int framedataslot = 0, anmoffset = 0;

    int start1 = (int)(maxfps * 5.75);
    int end1 = (int)(start1 - (maxfps * .75));
    int start2 = end1 - 1; int end2 = start2 - (maxfps * 4);
    int start3 = end2 - 1; int end3 = start3 - (maxfps / 2);
    int start4 = end3 - 1; int end4 = 0;

    if (_actiontime >= end1 && _actiontime <= start1)
    {   framedataslot = 45;
        anmoffset = (int)(((maxfps*.75)-(_actiontime-end1))/
                    ((maxfps*.75)/_spriteFrame[46].tile));
    }
    else if (_actiontime >= end2 && _actiontime <= start2)
    {   framedataslot = 47; }
    else if (_actiontime >= end3 && _actiontime <= start3)
    {   framedataslot = 48;
        anmoffset = (int)(((maxfps/2)-(_actiontime-end3))/
                    ((maxfps/2)/_spriteFrame[49].tile));
    }
    else if (_actiontime >= end4 && _actiontime <= start4)
    {   framedataslot = 50;
        anmoffset = (int)(((maxfps/2)-(_actiontime-end4))/
                    ((maxfps/2)/_spriteFrame[51].tile));
    }

    _frame  = _spriteFrame[framedataslot].tile + anmoffset;
    _txsize = _spriteFrame[framedataslot].width;
    _tysize = _spriteFrame[framedataslot].height;
}

/* ------------------------------------------------------------------------- *
 * void MapModeDoPhysics()                                                   *
 *                                                                           *
 * Handles the player's physics/movement when in overhead map mode.          *
 * ------------------------------------------------------------------------- */
void Player::mapModeDoPhysics()
{
    updateClipPoints();
    // - - - - - - - - - - - - MOVE PLAYER - - - - - - - - - - - -
    // --- Max speed ---
    if (_xv > 2.4) _xv = 2.4;
    if (_yv > 2) _yv = 2;
    if (_xv < -2.4) _xv = -2.4;
    if (_yv < -2) _yv = -2;
    
    // --- Move player ---
    _x += _xv;
    _y += _yv;
    
    // --- Friction ---
    _xv *= (Friction / 2);
    _yv *= (Friction / 2);
    if (_xv < .2 && _xv > 0) _xv = 0;
    if (_xv > -.2 && _xv < 0) _xv = 0;
    if (_yv < .2 && _yv > 0) _yv = 0;
    if (_yv > -.2 && _yv < 0) _yv = 0;

    // --- Map boundaries ---
    if (_x - 7 < 0) { _x = 7; _xv = 0; }
    if (_y - 7 < 0) { _y = 7; _yv = 0; }
    if (_x + 7 > mapWidth() * ts - 1) { _x = (mapWidth() * ts) - 1 - 7; }
    if (_y + 7 > mapHeight() * ts - 1) { _y = (mapHeight() * ts) - 1 - 7; }

    updateClipPoints();
    // - - - - - - - - - - - - - CLIPPING - - - - - - - - - - - - -
    
    if (!key[KEY_N]) mapModeClipping();
    updateClipPoints();
}

/* ------------------------------------------------------------------------- *
 * void mapModeClipping()                                                    *
 *                                                                           *
 * Performs clipping between player and the map. Calls other functions if    *
 * special tiles are touched. This clipping routine is just for overhead     *
 * map mode.                                                                 *
 * ------------------------------------------------------------------------- */
void Player::mapModeClipping()
{
    for (int c=1; c<=8; c++)
    {   switch (mapHitPoint(_clippoint[c].x, _clippoint[c].y))
        {   case WALL:
            mapModeHitWall(c, _clippoint[c].x, _clippoint[c].y); break;
            case FGWALL:
            mapModeHitWall(c, _clippoint[c].x, _clippoint[c].y); break;
        }
        updateClipPoints();
    }
}

/* ------------------------------------------------------------------------- *
 * void mapModeHitWall(int p, float hx, float hy)                            *
 *   p    : clippoint that hit the tile                                      *
 *   hx,hy: location of collision                                            *
 *                                                                           *
 * Performs clipping between player and a single wall tile. Uses the players *
 * hitpoint colliding with the tile to move him back outside of it again.    *
 * Used for overhead map mode only.                                          *
 * ------------------------------------------------------------------------- */
void Player::mapModeHitWall(int p, float hx, float hy)
{
    int side = 1;
    float ny, nx;
    
    if (p == 1)
    {   ny = (int)((hy - ((int)hy % ts)) + ts); side = UP; }
    else if (p == 2)
    {   ny = (int)((hy - ((int)hy % ts)) - 1); side = DOWN; }
    else if (p == 3 || p == 5 || p == 7 || p == 9)
    {   nx = (int)((hx - ((int)hx % ts)) + ts); side = RIGHT; }
    else if (p == 4 || p == 6 || p == 8 || p == 10)
    {   nx = (int)((hx - ((int)hx % ts)) - 1); side = LEFT; }
    
    switch (side)
    {   case UP: _yv = 0; _y = (int)(ny + 7) + 1; break;
        case DOWN: _yv = 0; _y = (int)(ny - 7); break;
        case LEFT: _xv = 0;
        if (p == 4 || p == 6 || p == 8 || p == 10) _x = (int)(nx - 7); break;
        case RIGHT: _xv = 0;
        if (p == 3 || p == 5 || p == 7 || p == 9) _x = (int)(nx + 7); break;
    }
}

/* ------------------------------------------------------------------------- *
 * void mapModeUpdateState()                                                 *
 *                                                                           *
 * Updates the player's current state, including animation frame, when in    *
 * overhead map mode.                                                        *
 * ------------------------------------------------------------------------- */
void Player::mapModeUpdateState()
{
    // --- player action ---
    int mx = (int)(_x / ts) + 1;
    int my = (int)(_y / ts) + 1;
    if (tileType(getMap(mx, my)) == WATER)
    {   if (!_scuba)
        {   _xv /= Friction / 2;
            _yv /= Friction / 2;
            _x -= _xv; _xv = 0;
            _y -= _yv; _yv = 0;
            showCantSwim();
            _action = MOVING;
        }
        else
        {   _action = SWIMMING; }
    }
    else
    {   _action = MOVING; }

    // --- player direction ---
    if (_action == MOVING || _action == SWIMMING)
    {   if (_xv == 0 && _yv < 0) _xdir = UP;
        if (_xv > 0 && _yv < 0) _xdir = UPRIGHT;
        if (_xv > 0 && _yv == 0) _xdir = RIGHT;
        if (_xv > 0 && _yv > 0) _xdir = DOWNRIGHT;
        if (_xv == 0 && _yv > 0) _xdir = DOWN;
        if (_xv < 0 && _yv > 0) _xdir = DOWNLEFT;
        if (_xv < 0 && _yv == 0) _xdir = LEFT;
        if (_xv < 0 && _yv < 0) _xdir = UPLEFT;
    }

    // - - - - - - - - - - Player Frame - - - - - - - - - -
    int framedataslot=0, anmoffset=0;
    switch (_xdir)
    {   case UP: framedataslot = 52; break;
        case DOWN: framedataslot = 53; break;
        case LEFT: framedataslot = 54; break;
        case RIGHT: framedataslot = 55; break;
        case UPLEFT: framedataslot = 56; break;
        case UPRIGHT: framedataslot = 57; break;
        case DOWNLEFT: framedataslot = 58; break;
        case DOWNRIGHT: framedataslot = 59; break;
    }

    if (_action == MOVING)
    {   if (_xv != 0 || _yv != 0)
        {   anmoffset = 1 + (int)(anmCounter() / _adelay) % 4;
            if (anmoffset == 1) anmoffset = 0;
            if (anmoffset == 2) anmoffset = 1;
            if (anmoffset == 3) anmoffset = 0;
            if (anmoffset == 4) anmoffset = 2;
        }
    }
    else if (_action == SWIMMING)
    {   framedataslot += 8;
        anmoffset = (int)(anmCounter() / _adelay) % 2;
    }

    _frame  = _spriteFrame[framedataslot].tile + anmoffset;
    _txsize = _spriteFrame[framedataslot].width;
    _tysize = _spriteFrame[framedataslot].height;
}

/* ------------------------------------------------------------------------- *
 * void updateClipPoints()                                                   *
 *                                                                           *
 * Calculates the points of the bounding box around the player.              *
 * ------------------------------------------------------------------------- */
void Player::updateClipPoints()
{
    /* 3--1--4
       |     |
       5     6
       |     |
       7     8
       |     |
       9--2-10 */

    if (moveMode() == MAPMODE)
    {
        _clippoint[ 3].x = _x - 7; _clippoint[ 3].y = _y - 7;
        _clippoint[ 1].x = _x;     _clippoint[ 1].y = _y - 7;
        _clippoint[ 4].x = _x + 7; _clippoint[ 4].y = _y - 7;
        _clippoint[ 6].x = _x + 7; _clippoint[ 6].y = _y - 3;
        _clippoint[ 8].x = _x + 7; _clippoint[ 8].y = _y + 3;
        _clippoint[10].x = _x + 7; _clippoint[10].y = _y + 7;
        _clippoint[ 2].x = _x;     _clippoint[ 2].y = _y + 7;
        _clippoint[ 9].x = _x - 7; _clippoint[ 9].y = _y + 7;
        _clippoint[ 7].x = _x - 7; _clippoint[ 7].y = _y + 3;
        _clippoint[ 5].x = _x - 7; _clippoint[ 5].y = _y - 3;
    }
    else /* moveMode() == JUMPMODE */
    {
        _clippoint[ 3].x = _x - _xsize/2; _clippoint[ 3].y = _y - _ysize/2;
        _clippoint[ 1].x = _x;            _clippoint[ 1].y = _y - _ysize/2;
        _clippoint[ 4].x = _x + _xsize/2; _clippoint[ 4].y = _y - _ysize/2;
        _clippoint[ 6].x = _x + _xsize/2; _clippoint[ 6].y = _y - _ysize/4;
        _clippoint[ 8].x = _x + _xsize/2; _clippoint[ 8].y = _y + _ysize/4;
        _clippoint[10].x = _x + _xsize/2; _clippoint[10].y = _y + _ysize/2;
        _clippoint[ 2].x = _x;            _clippoint[ 2].y = _y + _ysize/2;
        _clippoint[ 9].x = _x - _xsize/2; _clippoint[ 9].y = _y + _ysize/2;
        _clippoint[ 7].x = _x - _xsize/2; _clippoint[ 7].y = _y + _ysize/4;
        _clippoint[ 5].x = _x - _xsize/2; _clippoint[ 5].y = _y - _ysize/4;
    }
}

/* ------------------------------------------------------------------------- *
 * Accessors and Modifiers                                                   *
 *                                                                           *
 * A collection of accessors and modifiers for private variables.            *
 * ------------------------------------------------------------------------- */
void Player::setx(float x)            { _x=x; updateClipPoints(); }
void Player::sety(float y)            { _y=y; updateClipPoints(); }
void Player::setmapx(float x)         { _mapx=x; }
void Player::setmapy(float y)         { _mapy=y; }
void Player::setloc(float x, float y) { _x=x; _y=y; updateClipPoints(); }
void Player::setxdir(int xdir)        { _xdir=xdir; }
void Player::setmapxdir(int xdir)     { _mapxdir=xdir; }
void Player::setxsize(int xs)         { _xsize=xs; updateClipPoints(); }
void Player::setysize(int ys)         { _ysize=ys; updateClipPoints(); }
void Player::setxmax(float xm)        { _xmax=xm; }
void Player::setymax(float ym)        { _ymax=ym; }
void Player::setjumpheight(float jh)  { _jumpheight=jh; }
void Player::setinvincible(bool i)    { _invincible=i;  }
void Player::setlives(int l)          { _lives=l; }
void Player::setdead(bool d)          { _dead=d; }
void Player::setammo(int a)           { _ammo=a; }
void Player::setscore(int s)          { _score=s; }
void Player::setnextlifeat(int n)     { _nextlifeat=n; }
void Player::setdrops(int d)          { _drops=d; }
void Player::setancientsrescued(int a){ _ancientsrescued=a; }
void Player::setaim(int a)            { _aim=a; }
void Player::setholdingjump(bool h)   { _holdingjump=h; }
void Player::setstoppedjump(bool s)   { _stoppedjump=s; }
void Player::setkeyred()              { _keyred=true; }
void Player::setkeyyel()              { _keyyel=true; }
void Player::setkeyblu()              { _keyblu=true; }
void Player::setkeygrn()              { _keygrn=true; }
void Player::notIdle() { _idletime=0; }

float Player::x()      { return _x; }
float Player::y()      { return _y; }
float Player::mapx()   { return _mapx; }
float Player::mapy()   { return _mapy; }
int   Player::xdir()   { return _xdir; }
int   Player::mapxdir(){ return _mapxdir; }
bool  Player::dead()   { return _dead; }
int   Player::action() { return _action; }
bool  Player::invincible() { return _invincible; }
bool  Player::onground()   { return _onground; }
int   Player::onlift()     { return _onlift; }
int   Player::onball()     { return _onball; }
bool  Player::holdingjump(){ return _holdingjump; }
int   Player::lives()  { return _lives; }
int   Player::score()  { return _score; }
int   Player::nextlifeat() { return _nextlifeat; }
int   Player::ammo()   { return _ammo;  }
int   Player::drops()  { return _drops; }
int   Player::ancientsrescued() { return _ancientsrescued; }
bool  Player::keyred() { return _keyred; }
bool  Player::keyyel() { return _keyyel; }
bool  Player::keyblu() { return _keyblu; }
bool  Player::keygrn() { return _keygrn; }
bool  Player::scuba()  { return _scuba;  }