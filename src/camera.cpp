#include <allegro.h>
using namespace std;
#include "engine.h"
#include "graphicslib.h"
#include "player.h"
#include "camera.h"

/* ------------------------------------------------------------------------- *
 * Camera(Player* player)                                                    *
 *                                                                           *
 * Default constructor.                                                      *
 * ------------------------------------------------------------------------- */
Camera::Camera(Player* player)
: x(0), y(0), vx1(0), vy1(0), vx2(0), vy2(0), _player(player)
{ }

/* ------------------------------------------------------------------------- *
 * void tick()                                                               *
 *                                                                           *
 * Updates the camera corners to keep camera location in its bounds.         *
 * ------------------------------------------------------------------------- */
void Camera::tick()
{   
    // -- dont track player while dying --
    if (getPlayerAction() == DYING) return;
    if (getPlayerDead() == 1) return;

    // -- loose tracking box --
    if (_player->x() - x >  50) x = _player->x() - 50;
    if (_player->x() - x < -50) x = _player->x() + 50;
    if (_player->y() - y >  35) y = _player->y() - 35;
    if (_player->y() - y < -35) y = _player->y() + 35;
    
    // -- set upper left of camera --
    vx1 = x   - 160; vy1 = y   - 100;
    vx2 = vx1 + 319; vy2 = vy1 + 199;
    if (vx1 < 0) vx1 = 0;
    if (vy1 < 0) vy1 = 0;
    
    // -- set lower right of camera --
    vx2 = vx1 + 319;
    vy2 = vy1 + 199;
    if (vx2 > (ts * mapWidth()) - 1) vx2 = (ts * mapWidth())-1;
    if (vy2 > (ts * mapHeight()) - 1) vy2 = (ts * mapHeight())-1;
    
    // -- reset upper left with lower right of camera --
    vx1 = vx2 - 319;
    vy1 = vy2 - 199;
}

/* ------------------------------------------------------------------------- *
 * void focus()                                                              *
 *                                                                           *
 * Focuses the camera on the player.                                         *
 * ------------------------------------------------------------------------- */
void Camera::focus()
{   x = _player->x(); y = _player->y(); tick(); }

/* ------------------------------------------------------------------------- *
 * void move(int movx, int movy)                                             *
 *                                                                           *
 * Moves the camera a given number of pixels.                                *
 * ------------------------------------------------------------------------- */
void Camera::move(int movx, int movy)
{   x += movx; y += movy; }

/* ------------------------------------------------------------------------- *
 * bool inView(int x1, int y1, int x2, int y2)                               *
 *   x1,y1,x2,y2: rectangle corners to check                                 *
 *                                                                           *
 * Returns true if the rectangle intersects with the camera viewing area.    *
 * ------------------------------------------------------------------------- */
bool Camera::inView(float x1, float y1, float x2, float y2)
{  
    if (x2 >= vx1 && x1 <= vx2 && y2 >= vy1 && y1 <= vy2) return true;
    return false;
}

/* ------------------------------------------------------------------------- *
 * TEDCamera(Player* player)                                                 *
 *                                                                           *
 * Constructor for the TEDCamera class, a floating camera used by TED which  *
 * uses 640x480 resolution, zooms and isnt attached to the player.           *
 * ------------------------------------------------------------------------- */
TEDCamera::TEDCamera(Player* player)
: Camera(player), zoom(1.0)
{ }

/* ------------------------------------------------------------------------- *
 * void tick()                                                               *
 *                                                                           *
 * Updates the camera corners to keep camera location in its bounds.         *
 * ------------------------------------------------------------------------- */
void TEDCamera::tick()
{
    // -- bound camera focus --
    if (x < 320) x = 320;
    if (y < 240) y = 240;
    if (x > ((float)ts/zoom)*mapWidth()-1)
    {   x = ((float)ts/zoom)*mapWidth()-1; }
    if (y > ((float)ts/zoom)*mapHeight()-1)
    {   y = ((float)ts/zoom)*mapHeight()-1; }
    
    // -- set upper left of camera --
    vx1 = x   - 320; vy1 = y   - 240;
    vx2 = vx1 + 639; vy2 = vy1 + 479;
    if (vx1 < 0) vx1 = 0;
    if (vy1 < 0) vy1 = 0;
    
    // -- set lower right of camera --
    vx2 = vx1 + 639;
    vy2 = vy1 + 479;
    if (vx2 > ((float)ts / zoom) * mapWidth() - 1)
    {   vx2 = ((float)ts / zoom) * mapWidth() - 1; }
    if (vy2 > ((float)ts / zoom) * mapHeight() - 1)
    {   vy2 = ((float)ts / zoom) * mapHeight() - 1; }
}

/* ------------------------------------------------------------------------- *
 * void focus()                                                              *
 *                                                                           *
 * Focuses the camera on the player.                                         *
 * ------------------------------------------------------------------------- */
void TEDCamera::focus()
{   x = _player->x() / zoom; y = _player->y() / zoom; tick(); }

/* ------------------------------------------------------------------------- *
 * bool inView(int x1, int y1, int x2, int y2)                               *
 *   x1,y1,x2,y2: rectangle corners to check                                 *
 *                                                                           *
 * Returns true if the rectangle intersects with the camera viewing area.    *
 * ------------------------------------------------------------------------- */
bool TEDCamera::inView(float x1, float y1, float x2, float y2)
{
    x1 /= zoom; y1 /= zoom; x2 /= zoom; y2 /= zoom;  
    if (x2 >= vx1 && x1 <= vx2 && y2 >= vy1 && y1 <= vy2) return true;
    return false;
}

/* ------------------------------------------------------------------------- *
 * void zoomIn()                                                             *
 *                                                                           *
 * Changes the zoom factor for the camera to zoom in.                        *
 * ------------------------------------------------------------------------- */
void TEDCamera::zoomIn()
{
    if (zoom > .5) { zoom /= 2; x *= 2; y *= 2; tick(); }
}

/* ------------------------------------------------------------------------- *
 * void zoomOut()                                                            *
 *                                                                           *
 * Changes the zoom factor for the camera to zoom out.                       *
 * ------------------------------------------------------------------------- */
void TEDCamera::zoomOut()
{
    if (zoom < 4) { zoom *= 2; x /= 2; y /= 2; tick(); }
}
