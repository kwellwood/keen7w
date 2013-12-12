#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <allegro.h>
#include <almp3.h>
using namespace std;
#include "tedk7.h"

/* to do:
[ ] build TED with warnings enabled
[ ] Add background layer shading option (shade bg when shown with fg)
[x] move FG, BG buttons next to delete
[x] right click on tile in chooser to diplay tile number and type
 */

void main(int argc, char *argv[])
{
    for (int i=1; i<argc; i++)
    {   if (strcmp(argv[i], "-windowed") == 0) forcewindowed = true;
    }
    //forcewindowed = true;
    
    // --- Initialize Allegro ---
    allegro_init();
    
    // --- Initialize window ---
    set_window_title("TED");
    set_close_button_callback(closeButton);
    initGraphicsMode();
    Audio::init(); if (Audio::MusicVol == 0) Audio::toggleMusic();

    // --- Initialize engine and load data ---
    initEngine();
    menu = new Menu();
    Lift::init();
    Enemy::init();
    tilelib = new GraphicsLib(tileset);
    if (tilelib->error()) error(tilelib->errmsg());
    map = new Map(tilelib);
    if (!loadMap("default.map")) { shutdown; return; }
    
    mainloop();

    // --- Shut down and clear memory ---
    shutdown();
    return;    
}
END_OF_MAIN();

void closeButton()
{   quitgame = true; }
END_OF_FUNCTION(closeButton);

/* ------------------------------------------------------------------------- *
 * void mainloop()                                                           *
 *                                                                           *
 * Contains the main executing code for a game cycle.                        *
 * ------------------------------------------------------------------------- */
void mainloop()
{
    do {
        doTimers();
        trackMouse();
        if (timerTicked(FRAMETIMER))
        {
            doInput();
            //map->tick();
            //player->tick();
            //Lift::tickLifts();
            //Enemy::tickEnemies();
            camera->tick();
            drawScreen();
        }
    } while (!quitgame);
}

/* ------------------------------------------------------------------------- *
 * void doInput()                                                            *
 *                                                                           *
 * Polls the keyboard and handles all key presses.                           *
 * ------------------------------------------------------------------------- */
void doInput()
{
    if (key[KEY_UP])    camera->move(0, -8/camera->zoom);
    if (key[KEY_DOWN])  camera->move(0,  8/camera->zoom);
    if (key[KEY_LEFT])  camera->move(-8/camera->zoom, 0);
    if (key[KEY_RIGHT]) camera->move( 8/camera->zoom, 0);
    if (key[KEY_MINUS])  { camera->zoomOut(); while(key[KEY_MINUS]); }
    if (key[KEY_EQUALS]) { camera->zoomIn(); while(key[KEY_EQUALS]); }
    if (key[KEY_1]) { drawinfo.layer ^= BACKGROUND; while(key[KEY_1]); }
    if (key[KEY_2]) { drawinfo.layer ^= FOREGROUND; while(key[KEY_2]); }
    if (key[KEY_ESC]) setMode(TILE_MODE);
    if (key[KEY_T])
    {   int t = menu->chooseTile(drawinfo.tile);
        if (t >= 0) drawinfo.tile = t; }
    if (key[KEY_DEL])
    {   while (key[KEY_DEL]);
        if (drawinfo.mode != DEL_MODE) setMode(DEL_MODE);
        else setMode(TILE_MODE); }
    if (key[KEY_C])
    {   menu->toggleMouseCoordsType(); while (key[KEY_C]); }
    if (key[KEY_L])
    {   if (getMode() != LIFT_MODE) setMode(LIFT_MODE);
        else setMode(TILE_MODE);
        while (key[KEY_L]); }
    if (key[KEY_O])
    {   if (getMode() != LOCATR_MODE) setMode(LOCATR_MODE);
        else setMode(TILE_MODE);
        while (key[KEY_O]); }
    if (key[KEY_P])
    {   if (getMode() != PORTAL_MODE)
        {   setPortal(PLACE_ENTER); setMode(PORTAL_MODE); }
        else { setMode(TILE_MODE); }
        while (key[KEY_P]); }
    if (key[KEY_S])
    {   if (getMode() != SWTAG_MODE) setMode(SWTAG_MODE);
        else setMode(TILE_MODE);
        while (key[KEY_S]); }

    trackMouse();
    if (mouseLB())
    {
        // Ask menu to handle the click; false is returned if it didnt find
        // anything to do. Thats when we do the switch statement for painting
        // tiles and placing objects on the map
        if (!menu->leftClick(mouseX(), mouseY()) && mouseY() >= 10)
        {   switch (drawinfo.mode)
            {   // -- Draw a single tile --
                case TILE_MODE:
                paintTile(drawinfo.layer, mouseXTile(), mouseYTile(),
                    drawinfo.tile); break;
                // -- Draw a tile from a matrix --
                case MATRIX_MODE:
                paintMatrix(drawinfo.layer, mouseXTile(), mouseYTile(),
                    drawinfo.matrix); break;
                // -- Spawn an item object --
                case ITEM_MODE:
                createItem(mouseX(), mouseY(), drawinfo.item);
                waitForLB(); break;
                // -- Spawn an enemy object --
                case ENEMY_MODE:
                waitForLB();
                createEnemy(mouseX(), mouseY(), getDir(),
                    drawinfo.enemy, key[KEY_G]);
                waitForLB(); break;
                // -- Delete an object --
                case DEL_MODE:
                delObject(mouseX(), mouseY());
                waitForLB(); break;
                // -- Set the player start --
                case PLAYER_MODE:
                setPlayerStart(mouseX(), mouseY(), getDir(), key[KEY_G]);
                waitForLB(); break;
                case SWTAG_MODE:
                menu->promptSwitchTag(mouseXTile(), mouseYTile());
                waitForLB(); break;
                case PORTAL_MODE:
                createPortal(mouseX(), mouseY(), drawinfo.portal);
                if (drawinfo.portal == PLACE_ENTER)
                {   drawinfo.portal = PLACE_EXIT; }
                else if (drawinfo.portal == PLACE_EXIT)
                {   drawinfo.portal = PLACE_ENTER; }
                waitForLB(); break;
                case LOCATR_MODE:
                createLocator(mouseX(), mouseY(), drawinfo.lifttrack);
                waitForLB(); break;
                case LIFT_MODE:
                createLift(mouseX(), mouseY(), drawinfo.lifttrack);
                waitForLB(); break;
            }
        }
    }
    else
    {   menu->notMouseLB();
        if (mouseRB())
        {
            // Check if right-clicked on menu anywhere
            if (!menu->rightClick(mouseX(), mouseY()) && mouseY() >= 10)
            {   drawinfo.tile = grabTile(drawinfo.layer, mouseXTile(),
                    mouseYTile());
            }
            // If right clicked while placing portal, delete it
            if (drawinfo.portal == PLACE_EXIT)
            {   drawinfo.portal = PLACE_ENTER; Portal::deleteLast(); }
        }
    }
}

/* ------------------------------------------------------------------------- *
 * void drawScreen()                                                         *
 *                                                                           *
 * Refreshes the screen, drawing the map, objects, and menus.                *
 * ------------------------------------------------------------------------- */
void drawScreen()
{
    drawBox(0, 0, 639, 479, 14, FILLED);
    int ts2 = (int)((float)ts / camera->zoom);
    
    // ====== Translate camera rectangle ======
    int minx = (int)(camera->vx1 / ts2) + 1; int subx = (int)camera->vx1 % ts2;
    int miny = (int)(camera->vy1 / ts2) + 1; int suby = (int)camera->vy1 % ts2;
    int maxx = (int)(camera->vx2 / ts2) + 1;
    int maxy = (int)(camera->vy2 / ts2) + 1;

    if (drawinfo.layer & BACKGROUND)
    {
        // ========== Draw Tiles ==========
        for (int y=miny; y<=maxy; y++)
            for (int x=minx; x<=maxx; x++)
            {
                int px = (x - minx) * ts2 - subx;
                int py = 10 + ((y - miny) * ts2 - suby);
            
                int t = map->map(x, y);         // tiles on the map
                int t2 = map->mapFG(x, y);      //

                // --- draw background tile ---
                int al  = tilelib->anmLength(t);        // anmlength backg
                int al2 = tilelib->anmLength(t2);       // anmlength foreg
                int ad  = tilelib->anmDelay(t);         // anmdelay backg
                
                // change bg tile if its animated
                if (al > 0)       
                {   t += ((int)(anmcounter / ad) % al); }

                // change bg tile if its a switch based on switch status
                if (tilelib->tileType(t) == ASWITCH)
                {   t += (int)map->getSwitch(map->switchTag(x,y)); }

                drawSprite(t, px, py, camera->zoom);
                
                // darken the tile if the foregound is visible over it and
                // the option is selected
                if (drawinfo.darkenbg && drawinfo.layer & FOREGROUND)
                {   drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
                    drawBox(px, py, px+ts2 - 1, py+ts2 - 1, 14, FILLED);
                    solid_mode();
                }
                // ----------------------------
                
                // --- draw overlayed foreground tile ---
                if (t2 > 0 && (tilelib->tileType2(t2) == OVERLAY ||
                               tilelib->tileType2(t2) == MORPHTILE))
                    {
                        if (al2>0 && tilelib->tileType2(t2) != MORPHTILE)
                        {   t2 += (anmcounter/tilelib->anmDelay(t2))%al2; }
                        drawSprite(t2, px, py, camera->zoom);
                    }
                // --------------------------------------
            }
        // ================================
    }

    // --- Draw Objects ---
    Portal::drawPortals(camera->zoom, 0, 10);
    TEDLift::drawLifts(camera->zoom, 0, 10);
    TEDLift::drawLocators(camera->zoom, 0, 10);
    TEDItem::drawItems(camera->zoom, 0, 10);
    TEDEnemy::drawEnemies(camera->zoom, 0, 10);

    if (drawinfo.layer & FOREGROUND)
    {
        // ========== Draw Foreground Tiles ==========
        for (int y=miny; y<=maxy; y++)
            for (int x=minx; x<=maxx; x++)
            {
                int px = (x - minx) * ts2 - subx;
                int py = 10 + ((y - miny) * ts2 - suby);
            
                int t2 = map->mapFG(x, y);
                int t2type2 = tilelib->tileType2(t2);
            
                // Dont draw foreground tiles if they are:
                // type 14 (changing tile; overlayed). should be invisible.
                if (t2>0 && t2type2 != OVERLAY && t2type2 != MORPHTILE)
                {
                    if (tilelib->anmLength(t2) > 0)
                    {   t2 += (anmcounter/tilelib->anmDelay(t2)) %
                        tilelib->anmLength(t2); }
                    drawSprite(t2, px, py, camera->zoom);
                }
            }
        // ===========================================
    }

    if (drawinfo.mode == SWTAG_MODE)
    {
        // ========== Draw Nonzero Switchtags ==========
        for (int y=miny; y<=maxy; y++)
            for (int x=minx; x<=maxx; x++)
            {   
                int px = (x - minx) * ts2 - subx;
                int py = 10 + ((y - miny) * ts2 - suby);
                
                int sw = map->switchTag(x, y);
                
                // Draw a box around a tile with a nonzero switchtag
                // and draw the tag number inside
                if (sw != 0)
                {   drawBox(px, py, px+(int)((float)ts/camera->zoom)-1,
                            py+(int)((float)ts/camera->zoom)-1, 16, EMPTY);
                    if (camera->zoom < 1)
                    {   drawText(toStr(sw), px+2, py+2, 16); }
                }
            }
        // =============================================
    }

    // --- Draw player on top ---
    player->draw(camera->zoom, 0, 10);

    menu->draw(camera->zoom, 0, 10);
    
    blitToScreen();
}

/* ------------------------------------------------------------------------- *
 * void blitToScreen()                                                       *
 *                                                                           *
 * Blit the screen buffer (scrbuf) to the screen.                            *
 * ------------------------------------------------------------------------- */
void blitToScreen()
{
    if (graphicsmode == FULLSCREEN)
    {   blit(scrbuf, vidpage[drawpage], 0, 0, 0, 0, 640, 480);
        show_video_bitmap(vidpage[drawpage]);
        if (drawpage == 0) drawpage = 1; else drawpage = 0;
    }
    else
    {   blit(scrbuf, screen, 0, 0, 0, 0, 640, 480); }
}

/* ------------------------------------------------------------------------- *
 * void drawTile(int t, int x, int y [, float size])                         *
 *   t  : tile to draw                                                       *
 *   x,y: location of upper left                                             *
 *   size: ratio of drawing size to orig size                                *
 *                                                                           *
 * Draws the requested tile in the window in solid mode.                     *
 * ------------------------------------------------------------------------- */
void drawTile(int t, int x, int y)
{
    BITMAP* pic = tilelib->get(t);
    hideMouse();
    if (pic != NULL)
    {   blit(pic, scrbuf, 0, 0, x, y, pic->w, pic->h); }
    else
    {   rectfill(scrbuf, x, y, x+15, y+15, 14); }
    showMouse();
}

void drawTile(int t, int x, int y, float size)
{
    BITMAP* pic = tilelib->get(t);
    hideMouse();
    if (pic != NULL)
    {   stretch_blit(pic, scrbuf, 0, 0, pic->w, pic->h, x, y,
            (int)((float)pic->w * size), (int)((float)pic->h / size));
    }
    else
    {   rectfill(scrbuf, x, y,
            x+(int)(16.0/size)-1, y+(int)(16.0/size)-1, 14);
    }
    showMouse();
}

/* ------------------------------------------------------------------------- *
 * void drawSprite(int t, int x, int y [, float size])                       *
 * void drawSprite(int t, int x, int y [, int w, int h])                     * 
 *   t   : tile to draw                                                      *
 *   x,y : location of upper left                                            *
 *   size: ratio of drawing size to orig size                                *
 *   w,h : stretch tile to this width,height                                 *
 *                                                                           *
 * Draws the requested tile in the window in masked mode.                    *
 * ------------------------------------------------------------------------- */
void drawSprite(int t, int x, int y)
{
    BITMAP* pic = tilelib->get(t);
    hideMouse();
    if (pic != NULL)
    {   masked_blit(pic, scrbuf, 0, 0, x, y, pic->w, pic->h); }
    else
    {   rectfill(scrbuf, x, y, x+15, y+15, 14); }
    showMouse();
}

void drawSprite(int t, int x, int y, float size)
{
    BITMAP* pic = tilelib->get(t);
    hideMouse();
    if (pic != NULL)
    {   masked_stretch_blit(pic, scrbuf, 0, 0, pic->w, pic->h, x, y,
            (int)((float)pic->w / size), (int)((float)pic->h / size));
    }
    showMouse();
}

void drawSprite(int t, int x, int y, int w, int h)
{
    BITMAP* pic = tilelib->get(t);
    hideMouse();
    if (pic != NULL)
    {   masked_stretch_blit(pic, scrbuf, 0, 0, pic->w, pic->h, x, y, w, h); }
    showMouse();
}

void drawSprite(int t, int x, int y, int maxdim)
{
    BITMAP* pic = tilelib->get(t);
    hideMouse();
    if (pic != NULL)
    {   if (pic->w > pic ->h) 
        {   masked_stretch_blit(pic, scrbuf, 0, 0, pic->w, pic->h, x, y,
            maxdim, (int)(((float)maxdim / (float)pic->w) * (float)pic->h));
        }
        else
        {   masked_stretch_blit(pic, scrbuf, 0, 0, pic->w, pic->h, x, y,
            (int)(((float)maxdim / (float)pic->h) * (float)pic->w), maxdim);
        }
    }
    showMouse();
}

/* ------------------------------------------------------------------------- *
 * void drawBox(int x1, int y1, int x2, int y2, int col, int type)           *
 *   x1,y1,x2,y2: Corners of the rectangle                                   *
 *   col        : Color                                                      *
 *   type       : EMPTY or FILLED rectangle                                  *
 *                                                                           *
 * Draws a rectangle on the current video page.                              *
 * ------------------------------------------------------------------------- */
void drawBox(int x1, int y1, int x2, int y2, int col, int type)
{
    hideMouse();
    if (type == EMPTY) rect(scrbuf, x1, y1, x2, y2, col);
    else rectfill(scrbuf, x1, y1, x2, y2, col);
    showMouse();
}

/* ------------------------------------------------------------------------- *
 * void drawLine(int x1, int y1, int x2, int y2, int col)                    *
 *   x1,y1,x2,y2: End points of the line                                     *
 *   col        : Color                                                      *
 *                                                                           *
 * Draws a line on the current video page.                                   *
 * ------------------------------------------------------------------------- */
void drawLine(int x1, int y1, int x2, int y2, int col)
{   hideMouse(); line(scrbuf, x1, y1, x2, y2, col); showMouse(); }

/* ------------------------------------------------------------------------- *
 * void drawText(string text, int x, int y, int col)                         *
 *   text: text string to draw                                               *
 *   x,y : location of upper left of text                                    *
 *   col : color of text                                                     *
 *                                                                           *
 * Draws a text string using the current font onto the screen buffer.        *
 * ------------------------------------------------------------------------- */
void drawText(string text, int x, int y, int col)
{   hideMouse(); textout(scrbuf, font, text.c_str(), x, y, col); showMouse(); }

/* ------------------------------------------------------------------------- *
 * void drawBitmap(BITMAP* bitmap, int x, int y)                             *
 *   bitmap: bitmap object to draw                                           *
 *   x,y : location of upper left of bitmap                                  *
 *                                                                           *
 * Draws a bitmap object on screen buffer.                                   *
 * ------------------------------------------------------------------------- */
void drawBitmap(BITMAP* bitmap, int x, int y)
{   hideMouse();
    blit(bitmap, scrbuf, 0, 0, x, y, bitmap->w, bitmap->h);
    showMouse();
}

void grabBitmap(BITMAP* bitmap, int srcx, int srcy, int destx, int desty,
    int width, int height)
{   hideMouse();
    blit(scrbuf, bitmap, srcx, srcy, destx, desty, width, height);
    showMouse();
}

/* ------------------------------------------------------------------------- *
 * void paintTile(int l, int x, int y, int t)                                *
 *   l  : layer to draw on (BACKGROUND|FOREGROUND)                           *
 *   x,y: tile location to set                                               *
 *   t  : tile # painting with                                               *
 *                                                                           *
 * Sets a tile at x,y on layer l to tile t.                                  *
 * ------------------------------------------------------------------------- */
void paintTile(int l, int x, int y, int t)
{
    if (l & FOREGROUND) map->setMapFG(x, y, t);
    else if (l & BACKGROUND) map->setMap(x, y, t);
}

/* ------------------------------------------------------------------------- *
 * void paintMatrix(int l, int x, int y, int m)                              *
 *   l  :layer to draw on (BACKGROUND|FOREGROUND)                            *
 *   x,y: tile location to set                                               *
 *   t  : matrix # to paint with                                             *
 *                                                                           *
 * Sets a tile at x,y on layer l to the corresponding tile in matrix m.      *
 * ------------------------------------------------------------------------- */
void paintMatrix(int l, int x, int y, int m)
{
    // x,y = world coordinates; mx,my = matrix coordinates
    int mx = (x + matrices[m]->offsetX()) % matrices[m]->width();
    int my = (y + matrices[m]->offsetY()) % matrices[m]->height();
    if (mx == 0) mx = matrices[m]->width();
    if (my == 0) my = matrices[m]->height();
    if (mx < 0) mx += matrices[m]->width();
    if (my < 0) my += matrices[m]->height();
    if (l & FOREGROUND) map->setMapFG(x, y, matrices[m]->get(mx, my));
    else if (l & BACKGROUND) map->setMap(x, y, matrices[m]->get(mx, my));
}

/* ------------------------------------------------------------------------- *
 * int grabTile(int l, int x, int y)                                         *
 *   l: layer to grab from (BACKGROUND|FOREGROUND)                           *
 *   x,y: tile location to grab from                                         *
 *                                                                           *
 * Gets a tile at x,y on layer l to drawinfo.tile.                           *
 * ------------------------------------------------------------------------- */
int grabTile(int l, int x, int y)
{
    if (l & FOREGROUND) return map->mapFG(x, y);
    else if (l & BACKGROUND) return map->map(x, y);
}

/* ------------------------------------------------------------------------- *
 * void createItem(int x, int y, int i)                                      *
 *   x,y: location to place item                                             *
 *   i  : type of item                                                       *
 *                                                                           *
 * Creates a new item at the location. Gridsnap item if gridsnap is on.      *
 * ------------------------------------------------------------------------- */
void createItem(int x, int y, int i)
{
    x = (int)((float)(camera->vx1 + x) * camera->zoom);
    y = (int)((float)(camera->vy1 + y-10) * camera->zoom);
    if (drawinfo.gridsnap)
    {   x = ((int)(x / ts) + 1) * ts - (ts / 2);
        y = ((int)(y / ts) + 1) * ts - (ts / 2);
    }
    Item::createItem(x, y, i);
}

/* ------------------------------------------------------------------------- *
 * void createEnemy(int x, int y, int e)                                     *
 *   x,y: location to place item                                             *
 *   e  : type of enemy                                                      *
 *   dir: direction of enemy (LEFT/RIGHT)                                    *
 *   putOnGround: whether to drop the enemy to the ground or not             *
 *                                                                           *
 * Creates a new enemy at the location.                                      *
 * ------------------------------------------------------------------------- */
void createEnemy(int x, int y, int dir, int e, bool putOnGround)
{
    x = (int)((float)(camera->vx1 + x) * camera->zoom);
    y = (int)((float)(camera->vy1 + y-10) * camera->zoom);

    // put enemy on ground if putOnGround (probably pressing G key)
    Enemy::createEnemy(x, y, dir, e, putOnGround);
}

/* ------------------------------------------------------------------------- *
 * void createLocator(int x, int y int track)                                *
 *   x,y  : location to place the locator                                    *
 *   track: track to which the locator will belong (0-15)                    *
 *                                                                           *
 * Places a locator for a lift track on the map.                             *
 * ------------------------------------------------------------------------- */
void createLocator(int x, int y, int track)
{
    x = (int)((float)(camera->vx1 + x) * camera->zoom);
    y = (int)((float)(camera->vy1 + y-10) * camera->zoom);

    if (x < mapWidth()*ts && y < mapHeight()*ts)
    {
        int delay = 0, locksw = 0;
    
        menu->promptCreateLocator(delay, locksw);
        if (delay >= 0) { Lift::addLocator(x, y, track, delay, locksw); }
    }
}

/* ------------------------------------------------------------------------- *
 * void createLift(int x, int y, int track)                                  *
 *   x,y  : location of the lift                                             *
 *   track: track the lift will follow (0-15)                                *
 *                                                                           *
 * Places a locator for a lift track on the map.                             *
 * ------------------------------------------------------------------------- */
void createLift(int x, int y, int track)
{
    x = (int)((float)(camera->vx1 + x) * camera->zoom);
    y = (int)((float)(camera->vy1 + y-10) * camera->zoom);

    if (x < mapWidth()*ts && y < mapHeight()*ts)
    {   Lift::createLift(x, y, track); }
}

/* ------------------------------------------------------------------------- *
 * void createPortal(int x, int y, int part)                                 *
 *   x,y : location to place part of portal at                               *
 *   part: PLACE_ENTER for starting point | PLACE_EXIT for destination       *
 *                                                                           *
 * Places a portal on the map, either the start or the destination segment.  *
 * ------------------------------------------------------------------------- */
void createPortal(int x, int y, int part)
{
    x = (int)((float)(camera->vx1 + x) * camera->zoom);
    y = (int)((float)(camera->vy1 + y-10) * camera->zoom);
    
    // place portal section
    if (part == PLACE_ENTER)     { Portal::addPortal(x, y); }
    else if (part == PLACE_EXIT) { Portal::addDest(x, y);   }
}

/* ------------------------------------------------------------------------- *
 * void delObject(int x, int y)                                              *
 *   x,y: location to delete object at                                       *
 *                                                                           *
 * Checks for objects at the given location and deletes one if found.        *
 * Priority for deletion is: items, enemies, portals, lifts, locators.       *
 * ------------------------------------------------------------------------- */
void delObject(int x, int y)
{
    x = (int)((float)(camera->vx1 + x) * camera->zoom);
    y = (int)((float)(camera->vy1 + y-10) * camera->zoom);
    
    int i = Item::collideItem(x, y, x, y);
    int e = Enemy::collideEnemyID(x, y, x, y);
    int li = Lift::collideLifts((float)x, (float)y);
    int lo = Lift::atLocator(x, y);
    int p = Portal::atPortal((float)x, (float)y);

    if (e >= 0) { Enemy::delEnemy(e); }
    else if (i >= 0) { Item::delItem(i); }
    else if (li >= 0) { Lift::delLift(li); }
    else if (lo >= 0) { Lift::delLocator(lo); }
    else if (p >= 0) { Portal::delPortal(p); }
}

/* ------------------------------------------------------------------------- *
 * void setPlayerStart(int x, int y, int dir, bool putOnGround)              *
 *   x,y: location to place player                                           *
 *   dir: direction of player (LEFT/RIGHT)                                   *
 *   putOnGround: whether to drop the player to the ground or not            *
 *                                                                           *
 * Sets the player's location and direction. Used for creating the player's  *
 * starting point with TED.                                                  *
 * ------------------------------------------------------------------------- */
void setPlayerStart(int x, int y, int dir, bool putOnGround)
{
    x = (int)((float)(camera->vx1 + x) * camera->zoom);
    y = (int)((float)(camera->vy1 + y-10) * camera->zoom);
    
    // put player on ground if putOnGround (probably pressing G key)
    player->setStart(x, y, dir, putOnGround);
}

/* ------------------------------------------------------------------------- *
 * void doTimers()                                                           *
 *                                                                           *
 * Initializes several engine variables from a config file.                  *
 * ------------------------------------------------------------------------- */
void doTimers()
{
    if (timerTicked(ANMTIMER))
    {   anmcounter++; if (anmcounter > 840) anmcounter = 0; }
}

/* ------------------------------------------------------------------------- *
 * void initGraphicsMode()                                                   *
 *                                                                           *
 * Initializes the video mode to fullscreen or windowed. If FORCEWINDOWED    *
 * is defined to be true, engine will try using windowed mode only.          *
 * ------------------------------------------------------------------------- */
void initGraphicsMode()
{
    set_color_depth(BPP);
    if (forcewindowed)          // Only try windowed mode
    {   if (set_gfx_mode(GFX_DIRECTX_WIN, 640, 480, 0, 0) < 0)
        {   error("Unsupported graphics mode"); }
        if (set_display_switch_mode(SWITCH_PAUSE) < 0)
        {   error("Unsupported switch mode"); }
        graphicsmode = WINDOWED;
    }
    else
    {   // Try fullscreen mode first, then try windowed mode
        if (set_gfx_mode(GFX_DIRECTX, 640, 480, 0, 0) < 0)
        {   if (set_gfx_mode(GFX_DIRECTX_WIN, 640, 480, 0, 0) < 0)
            {   error("Unsupported graphics mode"); }
            else    // Windows graphics mode
            {   if (set_display_switch_mode(SWITCH_PAUSE) < 0)
                {   error("Unsupported switch mode"); }
                graphicsmode = WINDOWED;
            }
        }
        else    // Fullscreen graphics mode
        {   vidpage[0]=create_video_bitmap(640,480);clear_bitmap(vidpage[0]);
            vidpage[1]=create_video_bitmap(640,480);clear_bitmap(vidpage[1]);
            show_video_bitmap(vidpage[0]);
            drawpage = 1;
            graphicsmode = FULLSCREEN;
        }
    }
    scrbuf = create_bitmap(640, 480); clear_bitmap(scrbuf);
}

/* ------------------------------------------------------------------------- *
 * void initEngine()                                                         *
 *                                                                           *
 * Initializes several engine variables from a config file.                  *
 * ------------------------------------------------------------------------- */
void initEngine()
{
    player = new Player();
    camera = new TEDCamera(player);

    // --- Read ENGINE.CFG ---
    int      tmp;
    float    tmpf;
    string   tmpstr;
    ifstream file("ENGINE.CFG");
    if (!file.is_open())
    {   error("Can't open file ENGINE.CFG"); }
    file >> tmp;
    if (tmp != ENGINE_VERSION)
    {   file.close(); error("Invalid version of ENGINE.CFG"); }
    file >> tmpf; Player::Friction = tmpf; Enemy::Friction = tmpf;
    file >> tmpf; Player::Gravity = tmpf;
    file >> tileset;
    file >> tmpstr; //Audio::setSoundSet(tmpstr);
    file >> tmp; //Audio::setChannels(tmp);
    file >> tmpf; player->setxmax(tmpf);
    file >> tmpf; player->setymax(tmpf);
    file >> tmpf; player->setjumpheight(tmpf);
    file >> tmp; player->setxsize(tmp);
    file >> tmp; player->setysize(tmp);
    file.close();
    
    int nummatrices = 0;
    while (true)
    {   Matrix* m = new Matrix();
        if (m->load(".\\ted\\matrix."+toStr(nummatrices)))
        {   matrices.push_back(m); nummatrices++; }
        else { break; }
    }

    // --- Load the color palette ---
    packfile_password("billyblaze");
    DATAFILE* pal = load_datafile_object("engine.dat", "ENGINE_PAL");
    if (pal == NULL) error("Can't load ENGINE_PAL from engine.dat");
    set_palette((RGB*)pal->dat);
    // create the transparency table
    create_trans_table(&transtable, (RGB*)pal->dat, 128, 128, 128, NULL);
    color_map = &transtable;
    set_trans_blender(0, 0, 0, 128);
    unload_datafile_object(pal);

    // --- Load the font ---
    text_mode(-1);                  // draw text transparently
    DATAFILE* fnt = load_datafile_object("engine.dat", "ENGINE_FONT");
    if (fnt == NULL) error("Can't load ENGINE_FONT from engine.dat");
    font = (FONT*)fnt->dat;
    packfile_password(NULL);

    // --- Set up some variables ---
    setPlayerInvincible(true);
    quitgame = false;
    anmcounter = 0;
    drawinfo.gridsnap = false;
    drawinfo.darkenbg = false;
    drawinfo.mode = TILE_MODE;
    drawinfo.tile = 0;
    drawinfo.layer = BACKGROUND|FOREGROUND;
    drawinfo.matrix = -1;
    drawinfo.item = -1;
    drawinfo.enemy = -1;
    drawinfo.lifttrack = 0;
    drawinfo.portal = PLACE_ENTER;
    drawinfo.dir = LEFT;
    
    // --- Install timers and input devices ---
    for (int i=0; i<3; i++)
    {   timer[i].ticks=0; timer[i].flag=false; }
    install_int_ex(tickFrameTimer, BPS_TO_TIMER(maxfps));   // framerate
    install_int_ex(tickAnmTimer, BPS_TO_TIMER(anmfps));     // anmcounter rate
    install_int_ex(tickHiresTimer, MSEC_TO_TIMER(5));     // generic 5ms timer
    install_int_ex(tickMusic, BPS_TO_TIMER(5));           // tick mp3 music

    install_keyboard(); clear_keybuf();
    install_mouse(); showMouse();
}

/* ------------------------------------------------------------------------- *
 * bool loadMap(string mapfilename)                                          *
 *   mapfilename: filename of map relative to .\levels\                      *
 *                                                                           *
 * Loads a map from disk and focuses the camera. Returns true if map loading *
 * was successful.                                                           *
 * ------------------------------------------------------------------------- */
bool loadMap(string mapfilename)
{
    if (!map->load(".\\levels\\" + mapfilename)) return false;
    player->tick();
    camera->zoom = 1;
    camera->focus();
    camera->tick();
    return true;
}

/* ------------------------------------------------------------------------- *
 * void saveMap(string filename)                                             *
 *   filename: filename of map relative to .\levels\ to save                 *
 *                                                                           *
 * Saves a map to disk. There is no return value- it assumes the save is ok. *
 * ------------------------------------------------------------------------- */
void saveMap(string filename)
{   map->save(".\\levels\\" + filename); }

/* ------------------------------------------------------------------------- *
 * bool validFilename(string f)                                              *
 *   f: filename to check                                                    *
 *                                                                           *
 * Returns true only if 'f' is a valid dos-style file name.                  *
 * ------------------------------------------------------------------------- */
bool validFilename(string f)
{
    int dots = 0;
    for (int i=0; i<f.length(); i++)
    {   if ((f[i] < '0' || f[i] > '9') &&     // check invalid characters
            (f[i] < 'A' || f[i] > 'Z') &&
            (f[i] < 'a' || f[i] > 'z') &&
            (f[i] != '-') &&
            (f[i] != '_') &&
            (f[i] != '.')) return false;
        if (f[i] == '.') dots++;
    }
    if (dots > 1) return false;               // only one '.' allowed
    return true;
}

/* ------------------------------------------------------------------------- *
 * void trackMouse()                                                         *
 *                                                                           *
 * Copies the volatile allegro mouse variables into local engine variables.  *
 * ------------------------------------------------------------------------- */
void trackMouse()
{
    mousX = mouse_x;
    mousY = mouse_y;
    mousLB = mouse_b & 1;
    mousRB = mouse_b & 2;
}

/* ------------------------------------------------------------------------- *
 * Accessors and Modifiers                                                   *
 *                                                                           *
 * These methods access or modify engine variables, map or player objects.   *
 * Used to reduce class coupling.                                            *
 * ------------------------------------------------------------------------- */
string levelName() { return levelname; }
int  anmCounter()  { return anmcounter;   }
int  moveMode()    { return 0; }
void setDrawTile(int t) { drawinfo.tile = t; }
int  getDrawTile() { return drawinfo.tile; }
void toggleDrawLayer(int l) { drawinfo.layer ^= l; }
int  getDrawLayer() { return drawinfo.layer; }
int  getDir()       { return drawinfo.dir; }
void setDir(int d)  { drawinfo.dir = d; }
int  getMode()      { return drawinfo.mode;  }
void setMode(int m)
{   if (drawinfo.mode == PORTAL_MODE && m != PORTAL_MODE &&
        drawinfo.portal == PLACE_EXIT)
    {   drawinfo.portal = PLACE_ENTER; Portal::deleteLast(); }
    drawinfo.mode = m;
}
void setMatrix(int m) { drawinfo.matrix = m; drawinfo.mode = MATRIX_MODE; }
int  getMatrix()    { return drawinfo.matrix; }
void setItem(int i) { drawinfo.item = i; drawinfo.mode = ITEM_MODE; }
int  getItem()      { return drawinfo.item; }
void setEnemy(int e) { drawinfo.enemy = e; drawinfo.mode = ENEMY_MODE; }
int  getEnemy()      { return drawinfo.enemy; }
void setTrack(int t)  { drawinfo.lifttrack = t; }
int  getTrack()       { return drawinfo.lifttrack; }
void setPortal(int p) { drawinfo.mode = PORTAL_MODE; drawinfo.portal = p; }
int  getPortal()      { return drawinfo.portal; }
void toggleGridsnap() { drawinfo.gridsnap = !drawinfo.gridsnap; }
void toggleDarkenBG() { drawinfo.darkenbg = !drawinfo.darkenbg; }
bool getGridsnap()    { return drawinfo.gridsnap; }
bool getDarkenBG()    { return drawinfo.darkenbg; }
void setGameMode(int gm) { }
void setQuitGame()       { quitgame = true; }
bool getQuitGame()       { return quitgame; }
int  getTextLen(string str) { return text_length(font, str.c_str()); }
int  numMatrices() { return matrices.size(); }
int  getMatrixAt(int m, int x, int y) { return matrices[m]->get(x, y); }
int  getMatrixOffsetX(int m) { return matrices[m]->offsetX(); }
int  getMatrixOffsetY(int m) { return matrices[m]->offsetY(); }
void setMatrixOffset(int m, int offx, int offy)
{   matrices[m]->setOffset(offx, offy); }

int  mouseX() { return mousX; }
int  mouseY() { return mousY; }
int  mouseXTile()
{   return (int)floor((mouseX()+camera->vx1)/(ts/camera->zoom))+1; }
int  mouseYTile()
{   return (int)floor(((mouseY()-10)+camera->vy1)/(ts/camera->zoom))+1; }
bool mouseLB() { return mousLB; }
bool mouseRB() { return mousRB; }
bool mouseIn(int x1, int y1, int x2, int y2)
{ return mousX >= x1 && mousY >= y1 && mousX <= x2 && mousY <= y2; }
bool mouseIn(int area[])
{ return mousX >= area[0] && mousY >= area[1] && mousX <= area[2] &&
    mousY <= area[3]; }
void waitForLB()
{   while (mouse_b & 1)
    {   trackMouse(); if (timerTicked(FRAMETIMER)) blitToScreen(); }
}
void waitForRB()
{   while (mouse_b & 2) 
    {   trackMouse(); if (timerTicked(FRAMETIMER)) blitToScreen(); }
}

int    getNumTiles()    { return tilelib->size();       }
int    tileType(int t)  { return tilelib->tileType(t);  }
int    tileType2(int t) { return tilelib->tileType2(t); }
string tileDesc(int t)  { return tilelib->tileDesc(t);  }
string tileDesc2(int t) { return tilelib->tileDesc2(t); }
int    slopeMask(int t) { return tilelib->slopeMask(t); }

int  mapWidth()             { return map->width();     }
int  mapHeight()            { return map->height();    }
string mapGetMusic()        { return map->getMusic();  }
void mapSetMusic(string music) { map->setMusic(music); }
int  getMap(int x, int y)   { return map->map(x, y);   }
int  getMapFG(int x, int y) { return map->mapFG(x, y); }
int  getSwitchTag(int x, int y) { return map->switchTag(x, y); }
int  getSlopeHeight(int x, int y, int l) { return map->slopeHeight(x, y, l); }
int  mapHitPoint(float x, float y)       { return map->hitPoint(x, y);       }
void setMap(int x, int y, int t)   { map->setMap(x, y, t);   }
void setMapFG(int x, int y, int t) { map->setMapFG(x, y, t); }
void setSwitchTag(int x, int y, int sw) { map->setSwitchTag(x, y, sw); }
void mapReplaceTile(int t1, int t2) { map->replaceTile(t1, t2); }
void mapNew(int w, int h)
{ map->New(w, h); player->setStart(40, 40, RIGHT, false); camera->focus(); 
  camera->zoom = 1; }

void focusCamera()    { camera->focus();    }
float getCameraOrigX() { return camera->vx1; }
float getCameraOrigY() { return camera->vy1; }
bool onCamera(float x1, float y1, float x2, float y2)
{ return camera->inView(x1, y1, x2, y2); }

float getPlayerX()         { return player->x();          }
float getPlayerY()         { return player->y();          }
int  getPlayerXdir()       { return player->xdir();       }
int  getPlayerAction()     { return player->action();     }
bool getPlayerDead()       { return player->dead();       }
bool getPlayerInvincible() { return player->invincible(); }
int  getPlayerScore()      { return player->score();      }
int  getPlayerNextlifeat() { return player->nextlifeat(); }
int  getPlayerLives()      { return player->lives();      }
int  getPlayerAmmo()       { return player->ammo();       }
int  getPlayerDrops()      { return player->drops();      }
int  getPlayerOnlift()     { return player->onlift();     }
int  getPlayerOnBall()     { return player->onball();     }
int  getPlayerAncientsrescued()      { return player->ancientsrescued(); }
void setPlayerLoc(float x, float y)  { player->setloc(x, y);     }
void setPlayerInvincible(bool i)     { player->setinvincible(i); }
void setPlayerLives(int l)           { player->setlives(l);      }
void setPlayerDead(bool d)           { player->setdead(d);       }
void setPlayerAmmo(int a)            { player->setammo(a);       }
void setPlayerScore(int s)           { player->setscore(s);      }
void setPlayerNextlifeat(int n)      { player->setnextlifeat(n); }
void setPlayerDrops(int d)           { player->setdrops(d);      }
void setPlayerAncientsrescued(int a) { player->setancientsrescued(a); }
void setPlayerKeyred()           { player->setkeyred(); }
void setPlayerKeyblu()           { player->setkeyblu(); }
void setPlayerKeygrn()           { player->setkeygrn(); }
void setPlayerKeyyel()           { player->setkeyyel(); }
void togglePlayerJumpcheat() { player->setjumpcheat(!player->jumpcheat()); }
bool playerCollision(int x1, int y1, int x2, int y2)
{ return player->collision(x1,y1,x2,y2); }
void playerReadMap(ifstream &file) { player->readMap(file); }
void playerWriteMap(ofstream &file) { player->writeMap(file); }
void writePlayerSave(ofstream &file)
{ file.write((char*)player, sizeof(Player)); }
void readPlayerSave(ifstream &file)
{ file.read((char*)player, sizeof(Player)); }

/* ------------------------------------------------------------------------- *
 * void showMouse()                                                          *
 * void hideMouse()                                                          *
 *                                                                           *
 * Shows or hides the mouse from the buffer 'scrbuf'.                        *
 * ------------------------------------------------------------------------- */
void showMouse() { show_mouse(scrbuf); }
void hideMouse() { show_mouse(NULL);   }

/* ------------------------------------------------------------------------- *
 * bool timerTicked(int tmr)                                                 *
 *                                                                           *
 * Returns true if timer ticked since last query.                            *
 * ------------------------------------------------------------------------- */
bool timerTicked(int tmr)
{   bool i = timer[tmr].flag; timer[tmr].flag = false; return i; }

/* ------------------------------------------------------------------------- *
 * long timerTicks(int tmr)                                                  *
 *                                                                           *
 * Returns number of ticks counted since the timer was started.              *
 * ------------------------------------------------------------------------- */
long timerTicks(int tmr)
{   if (tmr >= 0 && tmr <= 2)
    {   return timer[tmr].ticks; }
    return 0;
}

/* ------------------------------------------------------------------------- *
 * void tick***Timer()                                                       *
 *                                                                           *
 * Executed by allegro on interrupt, these methods tick one of the timers.   *
 * ------------------------------------------------------------------------- */
void tickFrameTimer()
{   timer[FRAMETIMER].ticks++; timer[FRAMETIMER].flag = true; }
END_OF_FUNCTION(tickFrameTimer);

void tickAnmTimer()
{   timer[ANMTIMER].ticks++; timer[ANMTIMER].flag = true; }
END_OF_FUNCTION(tickAnmTimer);

void tickHiresTimer()
{   timer[HIRESTIMER].ticks++; timer[HIRESTIMER].flag = true; }
END_OF_FUNCTION(tickHiresTimer);

/* ------------------------------------------------------------------------- *
 * void tickMusic()                                                          *
 *                                                                           *
 * Executed by allegro on interrupt, polls the mp3 music if its playing.     *
 * ------------------------------------------------------------------------- */
void tickMusic()
{   if (Audio::Music != NULL) almp3_poll_mp3(Audio::Music); }
END_OF_FUNCTION(tickMusic);


/* ------------------------------------------------------------------------- *
 * void shutdown()                                                           *
 *                                                                           *
 * Deletes objects and closes allegro processes.                             *
 * ------------------------------------------------------------------------- */
void shutdown()
{
    hideMouse();
    
    Audio::shutdown();
    Enemy::clearEnemies();
    Item::clearItems();
    Lift::clearLifts();
    Portal::clearPortals();
    
    for (int m=0; m<matrices.size(); m++)
    {   delete matrices[m]; }
    
    unload_datafile_object(fnt);
    destroy_bitmap(vidpage[0]);
    destroy_bitmap(vidpage[1]);
    destroy_bitmap(scrbuf);
    delete camera;
    delete menu;
    delete map;
    delete tilelib;
}

/* ------------------------------------------------------------------------- *
 * void breakUpText(string text, string &line1, string &line2)               *
 *   text : The whole string to break up                                     *
 *   line1: Returns the first line of text                                   *
 *   line2: Returns the second line of text (if necessary)                   *
 *                                                                           *
 * If the pixel length of text is longer than maxlen, break it up into two   *
 * smaller lines (line1, line2), making line1 as close to length maxlen as   *
 * possible.                                                                 *
 * ------------------------------------------------------------------------- */
void breakUpText(string text, string &line1, string &line2, int maxlen)
{
    if (getTextLen(text) > maxlen)
    {   line2 = text; line1 = "";
        while (getTextLen(line1+line2.substr(0,line2.find(" ",0))) <= maxlen
               && line2.find(" ",0) != string::npos)
        {
            line1 += line2.substr(0, line2.find(" ", 0) + 1);
            line2 = line2.substr(line2.find(" ", 0) + 1);
        }
    }
    else
    {   line1 = text; line2 = ""; }
}

/* ------------------------------------------------------------------------- *
 * void waitForKey()                                                         *
 *                                                                           *
 * Wait for the user to press Space, Enter, or Ctrl.                         *
 * ------------------------------------------------------------------------- */
void waitForKey()
{
    while(key[KEY_ENTER] || key[KEY_SPACE] || key[KEY_LCONTROL] ||
          key[KEY_RCONTROL]);
    while(!key[KEY_ENTER] && !key[KEY_SPACE] && !key[KEY_LCONTROL] &&
          !key[KEY_RCONTROL]);
    while(key[KEY_ENTER] || key[KEY_SPACE] || key[KEY_LCONTROL] ||
          key[KEY_RCONTROL]);
}

/* ------------------------------------------------------------------------- *
 * int readqbInt(ifstream &file)                                             *
 * int readqbLong(ifstream &file)                                            *
 * float readqbFloat(ifstream &file)                                         *
 *   file: file stream object to read from                                   *
 *                                                                           *
 * Each of these three routines reads a QBasic data type from a file stream  *
 * and returns the value as a C++ data type (int or float.)                  *
 * ------------------------------------------------------------------------- */
int readqbInt(ifstream &file)
{   char i[2]; file.read(i, 2);
    return ((int)(unsigned char)i[0]|(int)((unsigned char)i[1]*256)); }

int readqbLong(ifstream &file) { int i; file.read((char*)&i, 4); return i; }

float readqbFloat(ifstream &file) { float i; file.read((char*)&i,4); return i; }

/* ------------------------------------------------------------------------- *
 * int readInt(ifstream &file)                                               *
 *   file: file stream object to read from                                   *
 *                                                                           *
 * Each of these routines reads a datatype from a file stream and returns    *
 * the value.                                                                *
 * ------------------------------------------------------------------------- */
int readInt(ifstream &file)
{  int i; file.read((char*)&i, sizeof(int)); return i; }
 
float readFloat(ifstream &file)
{   float i; file.read((char*)&i, sizeof(float)); return i; }

/* ------------------------------------------------------------------------- *
 * void writeInt(ifstream &file, int value)                                  *
 * void writeFloat(ifstream &file, float value)                              *
 *   file : file stream object to read from                                  *
 *   value: value to write                                                   *
 *                                                                           *
 * These methods write an integer or float to the given binary file stream.  *
 * ------------------------------------------------------------------------- */
void writeInt(ofstream &file, int value)
{   file.write((char*)&value, sizeof(int)); }

void writeFloat(ofstream &file, float value)
{   file.write((char*)&value, sizeof(float)); }

/* ------------------------------------------------------------------------- *
 * void error(string e)                                                      *
 *   e: A string containing the error message                                *
 *                                                                           *
 * Outputs the current error message in a popup.                             *
 * ------------------------------------------------------------------------- */
void error(string e)
{
    e = "Error: " + e;
    allegro_message(e.c_str());
    shutdown();
    exit(1);
}

/* ------------------------------------------------------------------------- *
 * string toStr(int n)                                                       *
 *   n: an integer to convert to string                                      *
 *                                                                           *
 * Converts an integer to a string.                                          *
 * ------------------------------------------------------------------------- */
string toStr(int n)
{
    char buffer[11];
    return string(itoa(n, buffer, 10));
}

/* ------------------------------------------------------------------------- *
 * Unused functions:                                                         *
 *   These are stubs for functions used in Keen7 but not TED.                *
 * ------------------------------------------------------------------------- */

bool getSwitch(int s) { return false; }
void setSwitch(int s, bool v) { }
void mapToggleBridge(int loc) { }
void mapUnlockDoor(int x, int y) { }
void startLevel(int level) { }
void setLevelComplete() { }
void showGotScubaGear() { }
void doAncientRescued() { }
void setGameOver() { }
void showCantSwim() { }

/* ------------------------------------------------------------------------- *
 * class Matrix:                                                             *
 *   This class holds a tile matrix for drawing complex tile arrangenets.    *
 *   It is used by k7ted only.                                               *
 * ------------------------------------------------------------------------- */
Matrix::Matrix()
:_grid(NULL), _offsetx(0), _offsety(0), _width(0), _height(0)
{ }

Matrix::~Matrix()
{   if (_grid != NULL) delete _grid; }

bool Matrix::load(string file)
{
    // Dont load another file once one has been loaded
    if (_grid != NULL) return false;
    
    ifstream f(file.c_str());
    if (!f.is_open()) { return false; }
    f >> _width;
    f >> _height;
    
    _grid = new int[_height * _width];
    
    for (int y=1; y<=_height; y++)
    {   for (int x=1; x<=_width; x++)
        {   if (f.bad()) { f.close(); return false; }
            int temp; f >> temp;
            _grid[(y * _width - _width) + x - 1] = temp;
        }
    }
    
    f.close();
    return true;
}

int Matrix::get(int x, int y)
{   if (x > 0 && x <= _width && y > 0 && y <= _height)
    {   return _grid[(y * _width - _width) + x - 1]; }
    return 0;
}

void Matrix::setOffset(int x, int y)
{   _offsetx = x % _width; _offsety = y % _height; }

int Matrix::offsetX() { return _offsetx; }
int Matrix::offsetY() { return _offsety; }
int Matrix::width()   { return _width; }
int Matrix::height()  { return _height; }

