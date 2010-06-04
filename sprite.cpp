#include <allegro.h>
using namespace std;
#include "engine.h"
#include "sprite.h"

/* ------------------------------------------------------------------------- *
 * static void addSprite(...)                                                *
 *   x,y   : location of sprite (in pixels)                                  *
 *   sframe: first frame in animation                                        *
 *   eframe: last frame in animation                                         *
 *   delay : animation frame delay (in ticks)                                *
 *   xsize,ysize: width and height of frames                                 *
 *   deadly: collision with player kills him                                 *
 *   repeat: continually repeat animation                                    *
 *   lifespan: life of sprite in ticks (only used if !repeat)                *
 *                                                                           *
 * Adds a new sprite into the collection and returns its ID.                 *
 * ------------------------------------------------------------------------- */
int Sprite::addSprite(float x, float y, int sframe, int eframe, int delay,
    int xsize, int ysize, bool deadly, bool repeat, int lifespan)
{
    Sprites.push_back(new Sprite(x, y, sframe, eframe, delay, xsize, ysize,
                             deadly, repeat, lifespan, NextID));
    NextID++;
    return NextID - 1;
}

/* ------------------------------------------------------------------------- *
 * static void killSprite(int s)                                             *
 *   s: sprite's position in the Sprites vector                              *
 *                                                                           *
 * Removes the sprite from Sprites collection by position in the vector.     *
 * ------------------------------------------------------------------------- */
void Sprite::killSprite(int s)
{
    vector<Sprite*>::iterator iter = Sprites.begin();
    while (s > 0) { iter++; s--; }
    delete (*iter); Sprites.erase(iter);
}

/* ------------------------------------------------------------------------- *
 * static void killSpriteByID(int id)                                        *
 *   id: an sprite's unique id                                               *
 *                                                                           *
 * Finds a sprite by its id and removes it from the collection.              *
 * ------------------------------------------------------------------------- */
void Sprite::killSpriteByID(int id)
{
    for (vector<Sprite*>::iterator i=Sprites.begin(); i!=Sprites.end(); i++)
    {   if ((*i)->_id == id) { delete (*i); Sprites.erase(i); return; } }
}

/* ------------------------------------------------------------------------- *
 * static Sprite* getSpriteByID(int id)                                      *
 *   id: a sprite's unique id                                                *
 *                                                                           *
 * Finds a sprite in the collection by its id and returns it.                *
 * ------------------------------------------------------------------------- */
Sprite* Sprite::getSpriteByID(int id)
{
    for (vector<Sprite*>::iterator i=Sprites.begin(); i!=Sprites.end(); i++)
    {   if ((*i)->_id == id) return (*i); }
    return NULL;
}

/* ------------------------------------------------------------------------- *
 * static void tickSprites()                                                 *
 *                                                                           *
 * Handles updating frames and deleting sprites for one game cycle.          *
 * ------------------------------------------------------------------------- */
void Sprite::tickSprites()
{
    for (int s=0; s<Sprites.size(); s++)
    {   Sprites[s]->tick();
        if (Sprites[s]->destroy()) { killSprite(s); s--; }
    }
}

/* ------------------------------------------------------------------------- *
 * static void drawSprites()                                                 *
 *                                                                           *
 * Draws all of the sprites in the collection.                               *
 * ------------------------------------------------------------------------- */
void Sprite::drawSprites()
{   for (int i=0; i<Sprites.size(); i++) Sprites[i]->draw(); }

/* ------------------------------------------------------------------------- *
 * static int collideSprite(int x1, int y1, int x2, int y2)                  *
 *   x1,y1,x2,y2: corners of the collision rectangle                         *
 *                                                                           *
 * Searchs the collection and returns position of the first sprite which     *
 * is deadly and intersects the given collision rectangle. Returns -1 if no  *
 * collisions.                                                               *
 * ------------------------------------------------------------------------- */
int Sprite::collideSprite(int x1, int y1, int x2, int y2)
{
    for (int s=0; s<Sprites.size(); s++)
    {   if (Sprites[s]->collision(x1, y1, x2, y2)) return s; }

    return -1;
}

/* ------------------------------------------------------------------------- *
 * static void clearSprites()                                                *
 *                                                                           *
 * Erases all sprites.                                                       *
 * ------------------------------------------------------------------------- */
void Sprite::clearSprites()
{   for (int s=0; s<Sprites.size(); s++) delete Sprites[s];
    Sprites.clear(); NextID = 0;
}

/* ------------------------------------------------------------------------- *
 * Sprite(...)                                                               *
 *   x,y   : location of sprite (in pixels)                                  *
 *   sframe: first frame in animation                                        *
 *   eframe: last frame in animation                                         *
 *   delay : animation frame delay (in ticks)                                *
 *   xsize,ysize: width and height of frames                                 *
 *   deadly: collision with player kills him                                 *
 *   repeat: continually repeat animation                                    *
 *   lifespan: life of sprite in ticks (only used if !repeat)                *
 *   id    : unique number identifying the sprite                            *
 *                                                                           *
 * The default constructor.                                                  *
 * ------------------------------------------------------------------------- */
Sprite::Sprite(float x, float y, int sframe, int eframe, int delay, int xsize,
               int ysize, bool deadly, bool repeat, int lifespan, int id)
:_x(x),_y(y),_sframe(sframe),_frame(sframe),_eframe(eframe),_delay(delay),
 _xsize(xsize),_ysize(ysize),_deadly(deadly),_repeat(repeat),
 _lifespan(lifespan),_id(id),_ticks(delay),_destroy(false)
{ }

/* ------------------------------------------------------------------------- *
 * void tick()                                                               *
 *                                                                           *
 * Handles updating the frame and lifespan for one game cycle.               *
 * ------------------------------------------------------------------------- */
void Sprite::tick()
{
    // -- do animation --
    _ticks--;
    if (_ticks == 0)
    {   _frame++; _ticks = _delay; }

    
    if (!_repeat)
    {   // no repeat and animation done
        if (_frame > _eframe)               // this sprite will be destroyed
        {   _destroy=true; return; }        // upon returning to tickSprites()
    }
    else
    {   // a repeating sprite
        if (_frame > _eframe) _frame = _sframe;
        
        if (_lifespan == 0)
        {   _destroy=true; return; }        // this sprite will be destroyed
        else if (_lifespan > 0)
        {   _lifespan--; }                  // decrease lifespan
    }
}

/* ------------------------------------------------------------------------- *
 * void draw()                                                               *
 *                                                                           *
 * Draws the sprite if it is on the screen.                                  *
 * ------------------------------------------------------------------------- */
void Sprite::draw()
{
    float x1 = _x - (_xsize / 2);
    float y1 = _y - (_ysize / 2);
    float x2 = _x + (_xsize / 2);
    float y2 = _y + (_ysize / 2);
    if (onCamera(x1, y1, x2, y2))
    {
        int x = (int)(_x - getCameraOrigX() - (_xsize / 2));
        int y = (int)(_y - (int)getCameraOrigY() - (_ysize / 2));
        drawSprite(_frame, x, y);
    }
}

/* ------------------------------------------------------------------------- *
 * bool collision(int x1, int y1, int x2, int y2)                            *
 *   x1,y1,x2,y2: collision rectangle                                        *
 *                                                                           *
 * Returns true if the given collision rectangle intersects the sprite.      *
 * ------------------------------------------------------------------------- */
bool Sprite::collision(int x1, int y1, int x2, int y2)
{
    // Only do collisions with deadly sprites
    if (!_deadly) return false;

    int sx1 = (int)(_x - (_xsize / 2)); int sy1 = (int)(_y - (_ysize / 2));
    int sx2 = (int)(_x + (_xsize / 2)); int sy2 = (int)(_y + (_ysize / 2));

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
 * void move(float movx, float movy)                                         *
 *   movx,movy: amounts to move on each axis                                 *
 *                                                                           *
 * Moves a sprite relative to its current position.                          *
 * ------------------------------------------------------------------------- */
void Sprite::move(float movx, float movy)
{   _x += movx; _y += movy; }

/* ------------------------------------------------------------------------- *
 * bool destroy()                                                            *
 *                                                                           *
 * Returns true if the sprite is done and needs to be destroyed.             *
 * ------------------------------------------------------------------------- */
bool Sprite::destroy()
{   return _destroy; }