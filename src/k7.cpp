#include <iostream>
#include <fstream>
#include <sstream>
#include <allegro.h>
#include <almp3.h>
using namespace std;
#include "k7.h"
#include "audio.h"
#include "lift.h"
#include "enemy.h"
#include "item.h"
#include "portal.h"
#include "sprite.h"
#include "shot.h"

/* to do now:
[ ] build Keen7 with warnings enabled
*/

/* to do later:
[ ] use rle_sprites for speed
[ ] fix enemy vertical jitter
[ ] fix sprite jitter
[ ] fix player jittering on lifts (player.cpp:draw():104)
[ ] actively check for ledge if hanging (hanging off changing tiles)
[ ] tweak climbing ledge animation
[ ] better text file parsing
 */

void main(int argc, char *argv[])
{
    for (int i=1; i<argc; i++)
    {   if (strcmp(argv[i], "-windowed") == 0) forcewindowed = true;
        if (strcmp(argv[i], "-nosound") == 0) Audio::Nosound = true;
    }
    //forcewindowed = true;
    
    // --- Initialize Allegro ---
    allegro_init();
    
    // --- Initialize window ---
    set_window_title("Commander Keen 7");
    set_close_button_callback(closeButton);
    initGraphicsMode();
    Audio::init();

    // --- Initialize engine and load data ---
    initEngine();
    menu = new Menu();
    scoreboard = new Scoreboard(player);
    Lift::init();
    Enemy::init();
    tilelib = new GraphicsLib(tileset);
    if (tilelib->error()) error(tilelib->errmsg());
    map = new Map(tilelib);
    loadLevel(curlevel);

    doTitleScreen();
    
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
        if (timerTicked(FRAMETIMER))
        {
            doInput();
            if (gameplay == RUNNING)
            {   if (movemode == JUMPMODE)
                {   map->tick();
                    player->tick();
                    Lift::tickLifts();
                    Sprite::tickSprites();
                    Shot::tickShots();
                    Enemy::tickEnemies();
                }
                else if (movemode == MAPMODE)
                {   player->tick();
                    Sprite::tickSprites();
                }
                camera->tick();
            }
            scoreboard->tick();
            drawScreen();
        }
        
        if (gamecompleteflag)                    // game has been won; stop
        {   gamecompleteflag=false; winGame(); } // play and show winGame
        if (levelsComplete[curlevel])           // end level with victory
        {   endLevel(SUCCESSFUL); }
        if (player->dead()) tryAgainMenu();
        if (gameoverflag)                        // game is all over; stop
        {   gameoverflag=false; endGame(); }     // play, return to main menu

    }while(!quitgame);
}

/* ------------------------------------------------------------------------- *
 * void doInput()                                                            *
 *                                                                           *
 * Polls the keyboard and handles all key presses.                           *
 * ------------------------------------------------------------------------- */
void doInput()
{
    if (gamemode == INGAME)
    {
        if (gameplay == RUNNING)
        {
            if (movemode == JUMPMODE)
            {
                if (key[KEY_UP])
                {
                    player->notIdle();
                    player->setaim(UP);
                    if (player->action() == CLIMBING) player->climbUpPole();
                    else if (player->action() == HANGING)
                    {   player->climbUpLedge(); }
                    else
                    {   player->tryGrabPole();
                        player->tryFlipSwitch();
                        player->tryTeleport();
                    }
                }
                if (key[KEY_DOWN])
                {
                    player->notIdle();
                    if (!player->onground() || player->action()==CLIMBING)
                    {   player->setaim(DOWN); }
                    if (player->action() == CLIMBING)
                    {   player->climbDownPole(); }
                    if (player->action()!=CLIMBING && player->onground())
                    {   player->tryGrabPole(); }
                }
                if (key[KEY_RCONTROL] || key[KEY_ALT])
                {   player->move(UP); }
                else if (player->holdingjump())       // not pressing jump
                {   player->setholdingjump(false);    //   do not allow
                    player->setstoppedjump(true);     //   jumping to continue
                }
                if (key[KEY_LCONTROL] || key[KEY_ALTGR])
                {   player->togglePogo(); }
                if (key[KEY_SPACE])
                {   player->shoot(); }
                if (key[KEY_PGUP]) camera->move(0, -2);
                if (key[KEY_PGDN]) camera->move(0, 2);
            }
            else if (movemode == MAPMODE)
            {
                if (key[KEY_UP]) player->move(UP);          // move up
                if (key[KEY_DOWN]) player->move(DOWN);      // move down
                if (key[KEY_LCONTROL] || key[KEY_RCONTROL])
                {   player->tryEnterLevel(); }              // enter a level
            }
            if (key[KEY_RIGHT]) player->move(RIGHT);    // move right
            if (key[KEY_LEFT])  player->move(LEFT);     // move left
            if (key[KEY_ENTER]) scoreboard->activateDropdown();
        }
        if (key[KEY_P] && !scoreboard->dropdownActive())
        {   if (gameplay == RUNNING) { gameplay = PAUSED; pausedingame = 1; }
            else { gameplay = RUNNING; pausedingame = 0; }
            while(key[KEY_P]);
        }
        if (key[KEY_ESC] && !scoreboard->dropdownActive())
        {   menu->handleKey(KEY_ESC); }
    }
    else if (gamemode == INMENU)
    {
        if (key[KEY_ESC])   menu->handleKey(KEY_ESC);
        if (key[KEY_UP])    menu->handleKey(KEY_UP);
        if (key[KEY_DOWN])  menu->handleKey(KEY_DOWN);
        if (key[KEY_ENTER]) menu->handleKey(KEY_ENTER);
    }

    if (key[KEY_F1])    menu->helpMenu();
    // Take screenshot (F9)
}

/* ------------------------------------------------------------------------- *
 * void loadLevel(int level)                                                 *
 *   level: a valid level number to load                                     *
 *                                                                           *
 * Loads a level by number and prepares for play.                            *
 * ------------------------------------------------------------------------- */
void loadLevel(int level)
{
    char temp[256];
    string mapfile;
    ifstream file("levels.cfg");
    if (!file.is_open()) error("Can't open file levels.cfg");
    file.getline(temp, 256);
    for (int i=1; i<= level; i++)
    {
        file.getline(temp, 256); mapfile = string(temp);
        file.getline(temp, 256); levelname = string(temp);  // not a local var
        file.getline(temp, 256);
    }
    file.close();

    if (!map->load(mapfile)) error("Can't load file "+mapfile);
    camera->focus();
    player->resetForLevel();
}

/* ------------------------------------------------------------------------- *
 * void startLevel(int level)                                                *
 *   level: a valid level number                                             *
 *                                                                           *
 * Exits the overhead map and enters a level.                                *
 * ------------------------------------------------------------------------- */
void startLevel(int level)
{
    if (levelsComplete[level]) return;          // already been beaten?

    long mark = timerTicks(HIRESTIMER) + 256;   // 1.28 seconds delay
                                              // (length of enter level sound)
    Audio::playSound(1);
    
    // Save overhead map location variables
    player->setmapx(player->x());
    player->setmapy(player->y());
    player->setmapxdir(player->xdir());

    curlevel = level;
    menu->drawLoadingBox(curlevel);
    loadLevel(curlevel);
    
    gameplay = RUNNING; pausedingame = 0;
    movemode = JUMPMODE;
    gamemode = INGAME;

    while(timerTicks(HIRESTIMER) < mark);
    Audio::playMusic();
}

/* ------------------------------------------------------------------------- *
 * string levelName(int level)                                               *
 *   level: level number                                                     *
 *                                                                           *
 * Reads and returns the name of the requested level from levels.cfg. If     *
 * no level number is given, the name of the current level is returned.      * 
 * ------------------------------------------------------------------------- */
string levelName(int level)
{
    char temp[256];
    string name;
    ifstream file("levels.cfg");
    if (!file.is_open()) error("Can't open file levels.cfg");
    file.getline(temp, 256);
    for (int i=1; i<= level; i++)
    {
        file.getline(temp, 256);
        file.getline(temp, 256); name = string(temp);
        file.getline(temp, 256);

    }
    file.close();

    return name;
}

string levelName()
{   return levelname; }

/* ------------------------------------------------------------------------- *
 * void drawScreen()                                                         *
 *                                                                           *
 * Refreshes the screen, drawing the map, objects, and menus.                *
 * ------------------------------------------------------------------------- */
void drawScreen()
{
    drawBox(0, 0, 319, 199, 14, FILLED);
    
    // ====== Translate camera rectangle ======
    int minx = (int)(camera->vx1 / ts) + 1; int subx = (int)camera->vx1 % ts;
    int miny = (int)(camera->vy1 / ts) + 1; int suby = (int)camera->vy1 % ts;
    int maxx = (int)(camera->vx2 / ts) + 1;
    int maxy = (int)(camera->vy2 / ts) + 1;

    // ========== Draw Tiles ==========
    for (int y=miny; y<=maxy; y++)
        for (int x=minx; x<=maxx; x++)
        {
            int px = (x - minx) * ts - subx;
            int py = (y - miny) * ts - suby;
            
            int t = map->map(x, y);         // tiles on the map
            int t2 = map->mapFG(x, y);      //

            // --- draw background tile ---
            int al  = tilelib->anmLength(t);        // anmlength background
            int al2 = tilelib->anmLength(t2);       // anmlength foreground
            int ad  = tilelib->anmDelay(t);         // anmdelay background
                
            // change bg tile if its animated
            if (al > 0)       
            {   t += ((int)(anmcounter / ad) % al); }

            // change bg tile if its a switch based on switch status
            if (tilelib->tileType(t) == ASWITCH)
            {   t += (int)map->getSwitch(map->switchTag(x,y)); }
                
            drawSprite(t, px, py);
            // ----------------------------
                
            // --- draw overlayed foreground tile ---
            if (t2 > 0 && (tilelib->tileType2(t2) == OVERLAY ||
                           tilelib->tileType2(t2) == MORPHTILE))
                {
                    if (al2>0 && tilelib->tileType2(t2) != MORPHTILE)
                    {   t2 += (anmcounter/tilelib->anmDelay(t2))%al2; }
                    drawSprite(t2, px, py);
                }
            // --------------------------------------
        }
    // ================================

    // --- Draw Objects ---
    Shot::drawShots();
    Item::drawItems();
    Lift::drawLifts();
    if (player->action() != DYING && gameplay != STOPPED) player->draw(1,0,0);
    Enemy::drawEnemies();
    Sprite::drawSprites();

    // ========== Draw Foreground Tiles ==========
    for (int y=miny; y<=maxy; y++)
        for (int x=minx; x<=maxx; x++)
        {
            int px = (x - minx) * ts - subx;
            int py = (y - miny) * ts - suby;
            
            int t2 = map->mapFG(x, y);
            int t2type2 = tilelib->tileType2(t2);
            
            // Dont draw foreground tiles if they are:
            // type 10 (exit level), type 12 (enter level), or
            // type 14 (changing tile; overlayed). these should be invisible.
            if (t2>0 && t2type2 != OVERLAY && t2type2 != ENDLEVEL &&
                t2type2 != ENTERLEVEL && t2type2 != MORPHTILE)
            {
                if (tilelib->anmLength(t2) > 0)
                {   t2 += (anmcounter/tilelib->anmDelay(t2)) %
                    tilelib->anmLength(t2); }
                drawSprite(t2, px, py);
            }
        }
    // ===========================================

    // --- Draw player on top if dead ---
    if (player->action() == DYING) player->draw(1,0,0);

    // --- Draw HUD Information ---
    if (gameplay != STOPPED) scoreboard->draw();
    if (gameplay == PAUSED)
    {   drawTile(88, 128, 80); drawText("Paused", 145, 96, 14); }
    if (scoreboard->dropdownActive()) scoreboard->drawDropdown();
    
    if (gamemode == INMENU) menu->draw();

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
    {   stretch_blit(scrbuf,vidpage[drawpage],0,0,320,200,0,40,640,400);
        show_video_bitmap(vidpage[drawpage]);
        if (drawpage == 0) drawpage = 1; else drawpage = 0;
    }
    else
    {   stretch_blit(scrbuf, screen, 0, 0, 320, 200, 0, 0, XRES, YRES); }
}

/* ------------------------------------------------------------------------- *
 * void drawTile(int t, int x, int y)                                        *
 *   t  : tile to draw                                                       *
 *   x,y: location of upper left                                             *
 *                                                                           *
 * Draws the requested tile in the window in solid mode.                     *
 * ------------------------------------------------------------------------- */
void drawTile(int t, int x, int y)
{
    BITMAP* pic = tilelib->get(t);
    if (pic != NULL)
    {   blit(pic, scrbuf, 0, 0, x, y, pic->w, pic->h); }
    else
    {   rectfill(scrbuf, x, y, x+15, y+15, 14); }
}

/* ------------------------------------------------------------------------- *
 * void drawSprite(int t, int x, int y [, float size])                       *
 *   t  : tile to draw                                                       *
 *   x,y: location of upper left                                             *
 *   size: ratio of drawing size to orig size                                *
 *                                                                           *
 * Draws the requested tile in the window in masked mode.                    *
 * ------------------------------------------------------------------------- */
void drawSprite(int t, int x, int y)
{
    BITMAP* pic = tilelib->get(t);
    if (pic != NULL)
    {   masked_blit(pic, scrbuf, 0, 0, x, y, pic->w, pic->h); }
    else
    {   rectfill(scrbuf, x, y, x+15, y+15, 14); }
}

void drawSprite(int t, int x, int y, float size)
{
    BITMAP* pic = tilelib->get(t);
    if (pic != NULL)
    {   masked_stretch_blit(pic, scrbuf, 0, 0, pic->w, pic->h, x, y,
            (int)((float)pic->w / size), (int)((float)pic->h / size));
    }
    else
    {   rectfill(scrbuf, x, y,
            x+(int)(16.0/size)-1, y+(int)(16.0/size)-1, 14);
    }
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
    if (type == EMPTY) rect(scrbuf, x1, y1, x2, y2, col);
    else rectfill(scrbuf, x1, y1, x2, y2, col);
}

/* ------------------------------------------------------------------------- *
 * void drawLine(int x1, int y1, int x2, int y2, int col)                    *
 *   x1,y1,x2,y2: End points of the line                                     *
 *   col        : Color                                                      *
 *                                                                           *
 * Draws a line on the current video page.                                   *
 * ------------------------------------------------------------------------- */
void drawLine(int x1, int y1, int x2, int y2, int col)
{   line(scrbuf, x1, y1, x2, y2, col); }

/* ------------------------------------------------------------------------- *
 * void drawText(string text, int x, int y, int col)                         *
 *   text: text string to draw                                               *
 *   x,y : location of upper left of text                                    *
 *   col : color of text                                                     *
 *                                                                           *
 * Draws a text string using the current font onto the screen buffer.        *
 * ------------------------------------------------------------------------- */
void drawText(string text, int x, int y, int col)
{   textout_ex(scrbuf, font, text.c_str(), x, y, col, -1); }

/* ------------------------------------------------------------------------- *
 * void drawBitmap(BITMAP* bitmap, int x, int y)                             *
 *   bitmap: bitmap object to draw                                           *
 *   x,y : location of upper left of bitmap                                  *
 *                                                                           *
 * Draws a bitmap object on screen buffer.                                   *
 * ------------------------------------------------------------------------- */
void drawBitmap(BITMAP* bitmap, int x, int y)
{   blit(bitmap, scrbuf, 0, 0, x, y, bitmap->w, bitmap->h); }

void grabBitmap(BITMAP* bitmap, int srcx, int srcy, int destx, int desty,
    int width, int height)
{   blit(scrbuf, bitmap, srcx, srcy, destx, desty, width, height); }

/* ------------------------------------------------------------------------- *
 * void tryAgainMenu()                                                       *
 *                                                                           *
 * Uses the menu's try again prompt to choose to end the level unsucessfully *
 * or try the level again.                                                   *
 * ------------------------------------------------------------------------- */
void tryAgainMenu()
{
    long mark = timerTicks(HIRESTIMER) + 200;
    while(mark > timerTicks(HIRESTIMER));
    
    if (menu->tryAgainMenu())
    // Try the level again
    {   player->setlives(player->lives()-1);
        loadLevel(curlevel); Audio::playMusic(); }
    else
    // Abort to map of shadowlands
    {   player->setlives(player->lives()-1); endLevel(UNSUCCESSFUL); }
}

/* ------------------------------------------------------------------------- *
 * void endLevel(bool successful)                                            *
 *   successful: player beat the level                                       *
 *                                                                           *
 * Player has exited the current level. Mark level complete if successful,   *
 * load the overhead map and change to movemode to MAPMODE.                  *
 * ------------------------------------------------------------------------- */
void endLevel(bool successful)
{
    long mark = timerTicks(HIRESTIMER) + 356;   // 1.78 seconds delay
                                                // (length of exit level sound)
    if (successful) Audio::playSound(2);
    
    curlevel = 2;
    menu->drawLoadingBox(curlevel);
    loadLevel(curlevel);
    prepareOverheadMap();

    // If the player has already been on the overhead map,
    // retrieve his previous location
    if (player->mapx() > 0 || player->mapy() > 0)
    {   player->setx(player->mapx());
        player->sety(player->mapy());
        player->setxdir(player->mapxdir());
    }
    
    gameplay = RUNNING; pausedingame = 0;
    movemode = MAPMODE;
    gamemode = INGAME;
    
    while (mark > timerTicks(HIRESTIMER));
    Audio::playMusic();
}

/* ------------------------------------------------------------------------- *
 * void prepareOverheadMap()                                                 *
 *                                                                           *
 * After loading the overhead map, this puts up the level-completed flag     *
 * sprites and changes any tiles attached to a completed level.              *
 * ------------------------------------------------------------------------- */
void prepareOverheadMap()
{
    int numlev;
    ifstream file("overheadmap.cfg");
    if (!file.is_open()) error("Can't open file overheadmap.cfg");
    file >> numlev;
    for (int i=1; i<=numlev; i++)
    {   int flagx=0, flagy=0, numtiles=0;
        file >> flagx;
        file >> flagy;
        file >> numtiles;

        if (levelsComplete[i] && i>2 && flagx >=0 && flagy >= 0)
        { Sprite::addSprite(flagx,flagy,1342,1345,6,26,32,false,true,-1); }
        
        for (int j=0; j<numtiles; j++)
        {
            int layer=0, x=0, y=0, newtile=0;
            file >> layer;
            file >> x;
            file >> y;
            file >> newtile;
            if (levelsComplete[i])
            {   if (layer == 0) setMap(x, y, newtile);
                if (layer == 1) setMapFG(x, y, newtile);
            }
        }
    }
    file.close();
}

/* ------------------------------------------------------------------------- *
 * void endGame()                                                            *
 *                                                                           *
 * Ends the current game and returns to the starting main menu.              *
 * ------------------------------------------------------------------------- */
void endGame()
{
    gameoverflag = false;
    curlevel = 1;
    loadLevel(curlevel);
    gameplay = STOPPED;
    gamemode = INMENU;
    movemode = JUMPMODE;
}

/* ------------------------------------------------------------------------- *
 * void winGame()                                                            *
 *                                                                           *
 * Displays the ending sequence and then ends the current game.              *
 * ------------------------------------------------------------------------- */
void winGame()
{
    menu->winGame();
    endGame();
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
    {   if (set_gfx_mode(GFX_DIRECTX_WIN, XRES, YRES, 0, 0) < 0)
        {   error("Unsupported graphics mode"); }
        if (set_display_switch_mode(SWITCH_PAUSE) < 0)
        {   error("Unsupported switch mode"); }
        graphicsmode = WINDOWED;
    }
    else
    {   // Try fullscreen mode first, then try windowed mode
        if (set_gfx_mode(GFX_DIRECTX, 640, 480, 0, 0) < 0)
        {   if (set_gfx_mode(GFX_DIRECTX_WIN, XRES, YRES, 0, 0) < 0)
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
    scrbuf = create_bitmap(320, 200); clear_bitmap(scrbuf);
}

/* ------------------------------------------------------------------------- *
 * void initEngine()                                                         *
 *                                                                           *
 * Initializes several engine variables from a config file.                  *
 * ------------------------------------------------------------------------- */
void initEngine()
{
    player = new Player();
    camera = new Camera(player);

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
    file >> totalAncients;
    file.close();

    // --- Create array to store completed levels ---
    file.open("levels.cfg");
    if (!file.is_open())
    {   error("Can't open file levels.cfg"); }
    file >> tmp;
    file.close();
    levelsComplete = new bool[tmp+1];
    for (int i=0; i<=tmp; i++)
    {   levelsComplete[i] = false; }

    // --- Load the color palette ---
    packfile_password("billyblaze");
    DATAFILE* palette = load_datafile_object("engine.dat", "ENGINE_PAL");
    if (palette == NULL) error("Can't load ENGINE_PAL from engine.dat");
    set_palette((RGB*)palette->dat);
    unload_datafile_object(palette);

    // --- Load the font ---
    DATAFILE* fnt = load_datafile_object("engine.dat", "ENGINE_FONT");
    if (fnt == NULL) error("Can't load ENGINE_FONT from engine.dat");
    font = (FONT*)fnt->dat;
    packfile_password(NULL);

    curlevel = 1;
    pausedingame = 0;
    movemode = JUMPMODE;
    gamemode = INMENU;
    gameplay = STOPPED;
    gameoverflag = false;
    gamecompleteflag = false;
    quitgame = false;
    anmcounter = 0;

    install_keyboard(); clear_keybuf();
    //set_keyboard_rate(250, 150);

    for (int i=0; i<3; i++)
    {   timer[i].ticks=0; timer[i].flag=false; }
    install_int_ex(tickFrameTimer, BPS_TO_TIMER(maxfps));   // framerate
    install_int_ex(tickAnmTimer, BPS_TO_TIMER(anmfps));   // anmcounter rate
    install_int_ex(tickHiresTimer, MSEC_TO_TIMER(5));   // generic 5ms timer
    install_int_ex(tickMusic, BPS_TO_TIMER(5));         // tick mp3 music
}

/* ------------------------------------------------------------------------- *
 * Accessors and Modifiers                                                   *
 *                                                                           *
 * These methods access or modify engine variables, map or player objects.   *
 * Used to reduce class coupling.                                            *
 * ------------------------------------------------------------------------- */
int  anmCounter()  { return anmcounter;   }
int  gameMode()    { return gamemode;     }
int  gamePlay()    { return gameplay;     }
int  moveMode()    { return movemode;     }
int  pausedInGame(){ return pausedingame; }
void setGameMode(int gm) { gamemode = gm;   }
void setGamePlay(int gp) { gameplay = gp;   }
void setMoveMode(int mm) { movemode = mm;   }
void setPausedingame(int p) { pausedingame = 0; }
void setLevelComplete()  { levelsComplete[curlevel] = true; }
void setLevelComplete(int l) { levelsComplete[l] = true; }
void setGameComplete()   { gamecompleteflag = true; }
void setGameOver()       { gameoverflag = true; }
void setQuitGame()       { quitgame = true; }
void setLevel(int l)     { curlevel = l; }
int  getLevel()          { return curlevel; }
void clearLevelsComplete()
{   ifstream file("levels.cfg"); int numlev; file >> numlev; file.close();
    for (int i=0; i<=numlev; i++) levelsComplete[i] = false; }
int  getLevelComplete(int level) { return levelsComplete[level]; }
int  getTextLen(string str) { return text_length(font, str.c_str()); }

int tileType(int t)  { return tilelib->tileType(t);  }
int tileType2(int t) { return tilelib->tileType2(t); }
int slopeMask(int t) { return tilelib->slopeMask(t); }

int mapWidth()             { return map->width();     }
int mapHeight()            { return map->height();    }
int getMap(int x, int y)   { return map->map(x, y);   }
int getMapFG(int x, int y) { return map->mapFG(x, y); }
int getSwitchTag(int x, int y) { return map->switchTag(x, y); }
int getSlopeHeight(int x, int y, int l) { return map->slopeHeight(x, y, l); }
int mapHitPoint(float x, float y)       { return map->hitPoint(x, y);       }
bool getSwitch(int s)      { return map->getSwitch(s);}
void setMap(int x, int y, int t)   { map->setMap(x, y, t);   }
void setMapFG(int x, int y, int t) { map->setMapFG(x, y, t); }
void setSwitchTag(int x, int y, int sw) { map->setSwitchTag(x, y, sw); }
void setSwitch(int s, bool v)      { map->setSwitch(s, v);   }
void mapToggleBridge(int loc)      { map->toggleBridge(loc); }
void mapUnlockDoor(int x, int y)   { map->unlockDoor(x, y);  }

void focusCamera()    { camera->focus();    }
float getCameraOrigX() { return camera->vx1; }
float getCameraOrigY() { return camera->vy1; }
bool onCamera(float x1, float y1, float x2, float y2)
    { return camera->inView(x1, y1, x2, y2); }

void showGotScubaGear() { menu->gotScubaGear(); }
void showCantSwim()     { menu->cantSwim(); }
void doAncientRescued() { menu->ancientRescued(); setLevelComplete();
    if (getPlayerAncientsrescued() == getNumAncients()) setGameComplete(); }
int  getNumAncients()   { return totalAncients; }

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
void setPlayerScuba(bool s)      { player->setScuba(s); }
void togglePlayerJumpcheat() { player->setjumpcheat(!player->jumpcheat()); }
bool playerCollision(int x1, int y1, int x2, int y2)
    { return player->collision(x1,y1,x2,y2); }
void playerReadMap(ifstream &file) { player->readMap(file); }
void playerWriteMap(ofstream &file) { }
void writePlayerSave(ofstream &file)
{ file.write((char*)player, sizeof(Player)); }
void readPlayerSave(ifstream &file)
{ file.read((char*)player, sizeof(Player)); }


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
 * void doTitleScreen()                                                      *
 *                                                                           *
 * Displays the title screen and waits for a key press.                      *
 * ------------------------------------------------------------------------- */
void doTitleScreen()
{
    drawTile(89, 0, 0); blitToScreen();
    waitForKey();
}

/* ------------------------------------------------------------------------- *
 * void shutdown()                                                           *
 *                                                                           *
 * Deletes objects and closes allegro processes.                             *
 * ------------------------------------------------------------------------- */
void shutdown()
{
    Audio::shutdown();
    Enemy::clearEnemies();
    Shot::clearShots();
    Item::clearItems();
    Sprite::clearSprites();
    Lift::clearLifts();
    Portal::clearPortals();
    unload_datafile_object(fnt);
    destroy_bitmap(vidpage[0]);
    destroy_bitmap(vidpage[1]);
    destroy_bitmap(scrbuf);
    delete scoreboard;
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

float readqbFloat(ifstream &file) { float i; file.read((char*)&i, 4); return i; }

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
