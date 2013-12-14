#include <math.h>
#include <unistd.h>
#include <allegro.h>
using namespace std;
#include "engine.h"
#include "audio.h"
#include "item.h"
#include "enemy.h"
#include "portal.h"
#include "tedmenu.h"

int* makeButton( int x1, int y1, int x2, int y2) {
     int* box = new int[4];
     box[0] = x1;
     box[1] = y1;
     box[2] = x2;
     box[3] = y2;
     return box;
}

/* ------------------------------------------------------------------------- *
 * Map()                                                                     *
 *   tilelib: the graphics library for this map                              *
 *                                                                           *
 * The default constructor.                                                  *
 * ------------------------------------------------------------------------- */
Menu::Menu()
:menustate(0)
{
    currentMap = "default.map";

    btns = new int*[57];
    btns[BTN_FILE]   = makeButton(  0,  0, 24,  9);
    btns[BTN_EDIT]   = makeButton( 25,  0, 54,  9);
    btns[BTN_CREATE] = makeButton( 55,  0, 94,  9);
    btns[BTN_MATRIX] = makeButton( 95,  0,139,  9);
    btns[BTN_ITEM]   = makeButton(140,  0,169,  9);
    btns[BTN_ENEMY]  = makeButton(170,  0,209,  9);
    btns[BTN_DELETE] = makeButton(210,  0,249,  9);
    btns[BTN_NEWMAP] = makeButton(  1, 11, 49, 18);    // under file
    btns[BTN_SAVE]   = makeButton(  1, 19, 49, 26);
    btns[BTN_SAVEAS] = makeButton(  1, 27, 49, 34);
    btns[BTN_LOAD]   = makeButton(  1, 35, 49, 42);
    btns[BTN_QUIT]   = makeButton(  1, 43, 49, 50);
    btns[BTN_MUSIC]  = makeButton( 26, 11,107, 18);    // under edit
    btns[BTN_PSTART] = makeButton( 26, 19,107, 26);
    btns[BTN_SWTAGS] = makeButton( 26, 27,107, 34);
    btns[BTN_TILREP] = makeButton( 26, 35,107, 42);
    btns[BTN_GRDSNP] = makeButton( 26, 43,107, 50);
    btns[BTN_DARKBG] = makeButton( 26, 51,107, 58);
    btns[BTN_LIFT]   = makeButton( 56, 11,104, 18);    // under create
    btns[BTN_LOCATR] = makeButton( 56, 19,104, 26);
    btns[BTN_PORTAL] = makeButton( 56, 27,104, 34);
    btns[BTN_NOITEM] = makeButton(141, 11,199, 18);    // under items
    btns[BTN_SODA]   = makeButton(141, 19,199, 26);
    btns[BTN_GUM]    = makeButton(141, 27,199, 34);
    btns[BTN_CHOCO]  = makeButton(141, 35,199, 42);
    btns[BTN_CANDY]  = makeButton(141, 43,199, 50);
    btns[BTN_DONUT]  = makeButton(141, 51,199, 58);
    btns[BTN_ICECRM] = makeButton(141, 59,199, 66);
    btns[BTN_FLASK]  = makeButton(141, 67,199, 74);
    btns[BTN_GUN]    = makeButton(141, 75,199, 82);
    btns[BTN_DROP]   = makeButton(141, 83,199, 90);
    btns[BTN_REDKEY] = makeButton(141, 91,199, 98);
    btns[BTN_BLUKEY] = makeButton(141, 99,199,106);
    btns[BTN_GRNKEY] = makeButton(141,107,199,114);
    btns[BTN_YELKEY] = makeButton(141,115,199,122);
    btns[BTN_NOENMY] = makeButton(171, 11,244, 18);    // under enemies
    btns[BTN_SLUG]   = makeButton(171, 19,244, 26);
    btns[BTN_BALL]   = makeButton(171, 27,244, 34);
    btns[BTN_MUSH]   = makeButton(171, 35,244, 42);
    btns[BTN_LICK]   = makeButton(171, 43,244, 50);
    btns[BTN_ARACH]  = makeButton(171, 51,244, 58);
    btns[BTN_WORM]   = makeButton(171, 59,244, 66);
    btns[BTN_SNAKE]  = makeButton(171, 67,244, 74);
    btns[BTN_OLDMAN] = makeButton(171, 75,244, 82);
    btns[BTN_SPEARU] = makeButton(171, 83,244, 90);
    btns[BTN_SPEARD] = makeButton(171, 91,244, 98);
    btns[BTN_SPEARL] = makeButton(171, 99,244,106);
    btns[BTN_SPEARR] = makeButton(171,107,244,114);
    btns[BTN_SHOTRU] = makeButton(171,115,244,122);
    btns[BTN_SHOTRD] = makeButton(171,123,244,130);
    btns[BTN_SHOTRL] = makeButton(171,131,244,138);
    btns[BTN_SHOTRR] = makeButton(171,139,244,146);
    btns[BTN_MROCK]  = makeButton(171,147,244,154);
    btns[BTN_CLOUD]  = makeButton(171,155,244,162);
    btns[BTN_COORDS] = makeButton(  0,  0, -1, -1);    // mouse coordinates
    btns[BTN_MAPBG]  = makeButton(250,  0,274,  9);    // bg layer button
    btns[BTN_MAPFG]  = makeButton(275,  0,299,  9);    // fg layer button

    objbar.x = 0; objbar.y = 479 - 20; objbar.mode = 0;
    objbar.dragx = -1; objbar.dragy = -1;
    tileChooserRow = 0;
    mouseCoordsType = PIXEL_COORDS;
}

/* ------------------------------------------------------------------------- *
 * ~Map()                                                                    *
 *                                                                           *
 * The destructor.                                                           *
 * ------------------------------------------------------------------------- */
Menu::~Menu()
{   delete btns; }

/* ------------------------------------------------------------------------- *
 * void leftClick(int x, int y)                                              *
 *   x,y: place the mouse button was pressed                                 *
 *                                                                           *
 * Handles a left mouse click on the menu.                                   *
 * ------------------------------------------------------------------------- */
bool Menu::leftClick(int x, int y)
{
    // ----------------- Dragging object bar ----------------
    if (objbar.dragx >= 0)
    {
        objbar.x = mouseX() - objbar.dragx;
        objbar.y = mouseY() - objbar.dragy;
        if (objbar.x < 0) objbar.x = 0;
        if (objbar.y < 10) objbar.y = 10;
        if (objbar.x + 100 > 639) objbar.x = 639 - 100;
        if (objbar.y + 20 > 479) objbar.y = 479 - 20;
    }
    
    // ------------------ Menubar Buttons -------------------
    else if (mouseIn(btns[BTN_COORDS]))
    {   waitForLB();
        if (mouseIn(btns[BTN_COORDS]))
        {   if (mouseCoordsType == TILE_COORDS) mouseCoordsType = PIXEL_COORDS;
            else mouseCoordsType = TILE_COORDS;
       }
    }
    
    // --------------- Map Tile Layer Buttons ---------------
    else if (mouseIn(btns[BTN_MAPBG]))
    {   waitForLB();
        if (mouseIn(btns[BTN_MAPBG])) toggleDrawLayer(BACKGROUND);
    }
    else if (mouseIn(btns[BTN_MAPFG]))
    {   waitForLB();
        if (mouseIn(btns[BTN_MAPFG])) toggleDrawLayer(FOREGROUND);
    }

    // ----------------- Main Pulldown Menus ----------------
    else if (mouseIn(btns[BTN_FILE]))
    {   waitForLB();
        if (mouseIn(btns[BTN_FILE]))
            if (menustate != MENU_FILE) menustate = MENU_FILE;
            else menustate = 0;
    }
    else if (mouseIn(btns[BTN_EDIT]))
    {   waitForLB();
        if (mouseIn(btns[BTN_EDIT]))
            if (menustate != MENU_EDIT) menustate = MENU_EDIT;
            else menustate = 0;
    }
    else if (mouseIn(btns[BTN_CREATE]))
    {   waitForLB();
        if (mouseIn(btns[BTN_CREATE]))
            if (menustate != MENU_CREATE) menustate = MENU_CREATE;
            else menustate = 0;
    }
    else if (mouseIn(btns[BTN_MATRIX]))
    {   waitForLB();
        if (mouseIn(btns[BTN_MATRIX]))
            if (menustate != MENU_MATRIX) menustate = MENU_MATRIX;
            else menustate = 0;
    }
    else if (mouseIn(btns[BTN_ITEM]))
    {   waitForLB();
        if (mouseIn(btns[BTN_ITEM]))
            if (menustate != MENU_ITEM) menustate = MENU_ITEM;
            else menustate = 0;
    }
    else if (mouseIn(btns[BTN_ENEMY]))
    {   waitForLB();
        if (mouseIn(btns[BTN_ENEMY]))
            if (menustate != MENU_ENEMY) menustate = MENU_ENEMY;
            else menustate = 0;
    }
    else if (mouseIn(btns[BTN_DELETE]))
    {   waitForLB();
        if (mouseIn(btns[BTN_DELETE]))
        {   menustate = 0;
            if (getMode() == DEL_MODE) setMode(TILE_MODE);
            else setMode(DEL_MODE);
        }
    }

    // ----------------- Pulldown Menu Options -----------------
    else if (menustate == MENU_FILE && mouseIn(btns[BTN_NEWMAP]))
    {   waitForLB();
        if (mouseIn(btns[BTN_NEWMAP]))
        {   menustate = 0;
            promptNewMap();
        }
    }
    else if (menustate == MENU_FILE && mouseIn(btns[BTN_SAVE]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SAVE]))
        {   saveMap(currentMap);
            drawBox(285, 235, 355, 244, 14, FILLED);
            drawBox(285, 235, 355, 244, 16, EMPTY);
            drawText("Map Saved", 287, 237, 70);
            blitToScreen();
            long mark = timerTicks(HIRESTIMER) + 200;
            while (timerTicks(HIRESTIMER) < mark) { usleep(1000); }
            menustate = 0;
        }
    }
    else if (menustate == MENU_FILE && mouseIn(btns[BTN_SAVEAS]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SAVEAS]))
        {   drawButton("Save file:", 283, 225, 357, 246, false);
            drawBox(285, 235, 355, 244, 14, FILLED);
            drawBox(285, 235, 355, 244, 16, EMPTY);
            blitToScreen();
            string file = getTextInput(287, 237, 70, 12, 70);
            if (file != "" && validFilename(file))
            {   if (getPortal() == PLACE_EXIT)  // delete an unfinished portal
                {   setPortal(PLACE_ENTER); Portal::deleteLast(); }
                saveMap(file); currentMap = file;
            }
            else if (file != "")
            {   // The filename was bad
                drawBox(285, 235, 355, 244, 14, FILLED);
                drawBox(285, 235, 355, 244, 16, EMPTY);
                drawText("Bad filename", 287, 237, 32);
                blitToScreen();
                long mark = timerTicks(HIRESTIMER) + 300;
                while (timerTicks(HIRESTIMER) < mark) { usleep(1000); }
            }
            menustate = 0;
        }
    }
    else if (menustate == MENU_FILE && mouseIn(btns[BTN_LOAD]))
    {   waitForLB();
        if (mouseIn(btns[BTN_LOAD]))
        {   drawButton("Load file:", 283, 225, 357, 246, false);
            drawBox(285, 235, 355, 244, 14, FILLED);
            drawBox(285, 235, 355, 244, 16, EMPTY);
            blitToScreen();
            string file = getTextInput(287, 237, 70, 12, 70);
            if (file != "" && !loadMap(file))
            {   // There was an error loading the map
                drawBox(285, 235, 355, 244, 14, FILLED);
                drawBox(285, 235, 355, 244, 16, EMPTY);
                drawText("Load failed", 287, 237, 32);
                blitToScreen();
                long mark = timerTicks(HIRESTIMER) + 300;
                while (timerTicks(HIRESTIMER) < mark) { usleep(1000); }
            }
            else if (file != "") { currentMap = file; }
            menustate = 0;
        }
    }
    else if (menustate == MENU_FILE && mouseIn(btns[BTN_QUIT]))
    {   waitForLB(); if (mouseIn(btns[BTN_QUIT])) setQuitGame(); }
    else if (menustate == MENU_EDIT && mouseIn(btns[BTN_MUSIC]))
    {   waitForLB();
        if (mouseIn(btns[BTN_MUSIC])) { chooseMusic(); menustate = 0; } }
    else if (menustate == MENU_EDIT && mouseIn(btns[BTN_PSTART]))
    {   waitForLB();
        if (mouseIn(btns[BTN_PSTART]))
        {   if (getMode()==PLAYER_MODE) { setMode(TILE_MODE); menustate = 0; }
            else { setMode(PLAYER_MODE); menustate = 0; } }
    }
    else if (menustate == MENU_EDIT && mouseIn(btns[BTN_SWTAGS]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SWTAGS]))
        {   if (getMode()==SWTAG_MODE) { setMode(TILE_MODE); menustate = 0; }
            else { setMode(SWTAG_MODE); menustate = 0; } }
    }
    else if (menustate == MENU_EDIT && mouseIn(btns[BTN_TILREP]))
    {   waitForLB();
        if (mouseIn(btns[BTN_TILREP])) { promptTileReplace(); menustate=0; } }
    else if (menustate == MENU_EDIT && mouseIn(btns[BTN_GRDSNP]))
    {   waitForLB();
        if (mouseIn(btns[BTN_GRDSNP])) { toggleGridsnap(); menustate=0; } }
    else if (menustate == MENU_EDIT && mouseIn(btns[BTN_DARKBG]))
    {   waitForLB();
        if (mouseIn(btns[BTN_DARKBG])) { toggleDarkenBG(); menustate=0; } }
    else if (menustate == MENU_CREATE && mouseIn(btns[BTN_LIFT]))
    {   waitForLB();
        if (mouseIn(btns[BTN_LIFT]))
        {   setMode(LIFT_MODE); menustate = 0; }
    }
    else if (menustate == MENU_CREATE && mouseIn(btns[BTN_LOCATR]))
    {   waitForLB();
        if (mouseIn(btns[BTN_LOCATR]))
        {   setMode(LOCATR_MODE); menustate = 0; }
    }
    else if (menustate == MENU_CREATE && mouseIn(btns[BTN_PORTAL]))
    {   waitForLB();
        if (mouseIn(btns[BTN_PORTAL]))
        {   setPortal(PLACE_ENTER); setMode(PORTAL_MODE); menustate = 0; }
    }

    else if (menustate == MENU_MATRIX &&
             mouseIn(btns[BTN_MATRIX][0], 10, btns[BTN_MATRIX][0]+37,
                11+18*(int)ceil((float)numMatrices()/2.0)))
    {   // Loop through each matrix button and check for a click
        for (int i=0; i<numMatrices(); i++)
        {   int l = (int)floor((float)i/2.0);
            if ((i % 2 == 0 &&
                 mouseIn(btns[BTN_MATRIX][0]+1, 11+l*18,
                         btns[BTN_MATRIX][0]+18, 11+l*18+17))
                ||
                (i % 2 == 1 &&
                 mouseIn(btns[BTN_MATRIX][0]+19, 11+l*18,
                         btns[BTN_MATRIX][0]+36, 11+l*18+17)))
            // Clicked on this matrix 'i'
            {   setMatrix(i); menustate = 0; }
        }
        waitForLB();
    }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_NOITEM]))
    {   waitForLB();
        if (mouseIn(btns[BTN_NOITEM])) {setMode(TILE_MODE); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_SODA]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SODA])) {setItem(SODA); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_GUM]))
    {   waitForLB();
        if (mouseIn(btns[BTN_GUM])) {setItem(GUM); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_CHOCO]))
    {   waitForLB();
        if (mouseIn(btns[BTN_CHOCO])) {setItem(CHOCOLATE); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_CANDY]))
    {   waitForLB();
        if (mouseIn(btns[BTN_CANDY])) {setItem(CANDY); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_DONUT]))
    {   waitForLB();
        if (mouseIn(btns[BTN_DONUT])) {setItem(DONUT); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_ICECRM]))
    {   waitForLB();
        if (mouseIn(btns[BTN_ICECRM])) {setItem(ICECREAM); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_FLASK]))
    {   waitForLB();
        if (mouseIn(btns[BTN_FLASK])) {setItem(FLASK); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_GUN]))
    {   waitForLB();
        if (mouseIn(btns[BTN_GUN])) {setItem(GUN); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_DROP]))
    {   waitForLB();
        if (mouseIn(btns[BTN_DROP])) {setItem(DROP); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_REDKEY]))
    {   waitForLB();
        if (mouseIn(btns[BTN_REDKEY])) {setItem(REDKEY); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_BLUKEY]))
    {   waitForLB();
        if (mouseIn(btns[BTN_BLUKEY])) {setItem(BLUKEY); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_GRNKEY]))
    {   waitForLB();
        if (mouseIn(btns[BTN_GRNKEY])) {setItem(GRNKEY); menustate=0;} }
    else if (menustate == MENU_ITEM && mouseIn(btns[BTN_YELKEY]))
    {   waitForLB();
        if (mouseIn(btns[BTN_YELKEY])) {setItem(YELKEY); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_NOENMY]))
    {   waitForLB();
        if (mouseIn(btns[BTN_NOENMY])) {setMode(TILE_MODE); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SLUG]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SLUG])) {setEnemy(SLUG); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_BALL]))
    {   waitForLB();
        if (mouseIn(btns[BTN_BALL])) {setEnemy(BOUNDER); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_MUSH]))
    {   waitForLB();
        if (mouseIn(btns[BTN_MUSH])) {setEnemy(MUSHROOM); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_LICK]))
    {   waitForLB();
        if (mouseIn(btns[BTN_LICK])) {setEnemy(LICK); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_ARACH]))
    {   waitForLB();
        if (mouseIn(btns[BTN_ARACH])) {setEnemy(ARACHNUT); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_WORM]))
    {   waitForLB();
        if (mouseIn(btns[BTN_WORM])) {setEnemy(WORM); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SNAKE]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SNAKE])) {setEnemy(SNAKE); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_OLDMAN]))
    {   waitForLB();
        if (mouseIn(btns[BTN_OLDMAN])) {setEnemy(ANCIENT); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SPEARU]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SPEARU])) {setEnemy(SPEARUP); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SPEARD]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SPEARD])) {setEnemy(SPEARDOWN); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SPEARL]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SPEARL])) {setEnemy(SPEARLEFT); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SPEARR]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SPEARR])) {setEnemy(SPEARRIGHT); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SHOTRU]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SHOTRU])) {setEnemy(SHOOTERUP); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SHOTRD]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SHOTRD])) {setEnemy(SHOOTERDOWN); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SHOTRL]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SHOTRL])) {setEnemy(SHOOTERLEFT); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_SHOTRR]))
    {   waitForLB();
        if (mouseIn(btns[BTN_SHOTRR])) {setEnemy(SHOOTERRIGHT); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_MROCK]))
    {   waitForLB();
        if (mouseIn(btns[BTN_MROCK])) {setEnemy(MIMROCK); menustate=0;} }
    else if (menustate == MENU_ENEMY && mouseIn(btns[BTN_CLOUD]))
    {   waitForLB();
        if (mouseIn(btns[BTN_CLOUD])) {setEnemy(CLOUD); menustate=0;} }

    // ----------------- Clicked on Object Bar -----------------
    else if (mouseIn(objbar.x, objbar.y, objbar.x+100, objbar.y+20))
    {   menustate = 0;
        // check for special buttons on objbar depending on mode
        if (getMode() == TILE_MODE && 
            mouseIn(objbar.x+2, objbar.y+2, objbar.x+17, objbar.y+17))
        {   waitForLB();
            if (mouseIn(objbar.x+2, objbar.y+2, objbar.x+17, objbar.y+17))
            {   int t = chooseTile(getDrawTile());
                if (t >= 0) setDrawTile(t); }
        }
        else if (getMode() == MATRIX_MODE &&
            mouseIn(objbar.x+54, objbar.y+10, objbar.x+74, objbar.y+19))
        {   waitForLB();
            if (mouseIn(objbar.x+54, objbar.y+10, objbar.x+74, objbar.y+19))
            {   promptMatrixOffsetX(); }
        }
        else if (getMode() == MATRIX_MODE &&
            mouseIn(objbar.x+79, objbar.y+10, objbar.x+99, objbar.y+19))
        {   waitForLB();
            if (mouseIn(objbar.x+79, objbar.y+10, objbar.x+99, objbar.y+19))
            {   promptMatrixOffsetY(); }
        }
        else if ((getMode() == ENEMY_MODE || getMode() == PLAYER_MODE) &&
            mouseIn(objbar.x+66,objbar.y+1,objbar.x+82,objbar.y+11))
        {   waitForLB();
            if (mouseIn(objbar.x+66,objbar.y+1,objbar.x+82,objbar.y+11))
            {   setDir(LEFT); }
        }
        else if ((getMode() == ENEMY_MODE || getMode() == PLAYER_MODE) &&
            mouseIn(objbar.x+83,objbar.y+1,objbar.x+99,objbar.y+11))
        {   waitForLB();
            if (mouseIn(objbar.x+83,objbar.y+1,objbar.x+99,objbar.y+11))
            {   setDir(RIGHT); }
        }
        else if ((getMode() == LOCATR_MODE || getMode() == LIFT_MODE) &&
            mouseIn(objbar.x+66,objbar.y+9,objbar.x+82,objbar.y+19))
        {   waitForLB();
            if (mouseIn(objbar.x+66,objbar.y+9,objbar.x+82,objbar.y+19))
            {   if (getTrack()==0) setTrack(15); else setTrack(getTrack()-1); }
        }
        else if ((getMode() == LOCATR_MODE || getMode() == LIFT_MODE) &&
            mouseIn(objbar.x+83,objbar.y+9,objbar.x+99,objbar.y+19))
        {   waitForLB();
            if (mouseIn(objbar.x+83,objbar.y+9,objbar.x+99,objbar.y+19))
            {   if (getTrack()==15) setTrack(0); else setTrack(getTrack()+1); }
        }
        else
        {   // ---- Begin dragging the bar ----
            objbar.dragx = mouseX() - objbar.x;
            objbar.dragy = mouseY() - objbar.y;
        }
    }
    
    // ----------------- Didnt click on anything -----------------
    else
    {   menustate = 0;
        if (!mouseIn(0, 0, 639, 9)) return false;
    }
    return true;
}

/* ------------------------------------------------------------------------- *
 * void notMouseLB()                                                         *
 *                                                                           *
 * Handles the menu actions for the left mouse button being up. Not much to  *
 * do except for the object bar.                                             *
 * ------------------------------------------------------------------------- */
void Menu::notMouseLB()
{
    objbar.dragx = -1; objbar.dragy = -1;
}

/* ------------------------------------------------------------------------- *
 * bool rightClick(int x, int y)                                             *
 *   x,y: place the mouse button was pressed                                 *
 *                                                                           *
 * Handles a right mouse click on the menu. Returns true if an action was    *
 * taken in the menu.                                                        *
 * ------------------------------------------------------------------------- */
bool Menu::rightClick(int x, int y)
{
    if (menustate != 0) { menustate = 0; waitForRB(); return true; }
    if (getMode() != TILE_MODE) {setMode(TILE_MODE); waitForRB(); return true;}
    return false;
}

/* ------------------------------------------------------------------------- *
 * void draw(float zoom, int vpx, int vpy)                                   *
 *   zoom: zoom level of the camera                                          *
 *   vpx,vpy: world viewport offset                                          *
 *                                                                           *
 * Draws the whole menu in its current state.                                *
 * ------------------------------------------------------------------------- */
void Menu::draw(float zoom, int vpx, int vpy)
{
    // --- top main bar ---
    drawBox(0, 0, 639, 9, 22, FILLED);
    drawLine(0, 9, 639, 9, 26);

    // --- dragable object bar ---
    drawObjectBar();

    // --- Dropdown menu buttons ---
    drawButton("File", btns[BTN_FILE], menustate == MENU_FILE);
    drawButton("Edit", btns[BTN_EDIT], menustate == MENU_EDIT);
    drawButton("Create", btns[BTN_CREATE], menustate == MENU_CREATE);
    drawButton("Matrices", btns[BTN_MATRIX], menustate == MENU_MATRIX);
    drawButton("Items", btns[BTN_ITEM], menustate == MENU_ITEM,
        getMode() == ITEM_MODE && menustate != MENU_ITEM);
    drawButton("Enemies", btns[BTN_ENEMY], menustate == MENU_ENEMY,
        getMode() == ENEMY_MODE && menustate != MENU_ENEMY);
    drawButton("Delete", btns[BTN_DELETE], false, getMode() == DEL_MODE);

    // --- Current level being edited ---
    drawText(currentMap, 635 - getTextLen(currentMap), 1, 14);

    // --- Draw mouse location in world coordinates ---
    int mousex = max((int)((float)(getCameraOrigX() + mouseX()-vpx) * zoom), 0);
    int mousey = max((int)((float)(getCameraOrigY() + mouseY()-vpy) * zoom), 0);
    if (mouseCoordsType == TILE_COORDS)
    {   mousex = (int)ceil((float)mousex / ts);
        mousey = (int)ceil((float)mousey / ts);
    }
    string mousecoords = "("+toStr(mousex)+", "+toStr(mousey)+")";
    btns[BTN_COORDS][0] = 635-getTextLen(currentMap)-10-getTextLen(mousecoords);
    btns[BTN_COORDS][1] = 1;
    btns[BTN_COORDS][2] = btns[BTN_COORDS][0] + getTextLen(mousecoords);
    btns[BTN_COORDS][3] = btns[BTN_COORDS][1] + 7;
    drawText(mousecoords, btns[BTN_COORDS][0], btns[BTN_COORDS][1],
        14 + (mouseCoordsType == TILE_COORDS));     // black for PIXEL_COORDS,
                                                    // white for TILE_COORDS
    // --- Map drawing layer buttons ---
    drawButton(" BG", btns[BTN_MAPBG], getDrawLayer() & BACKGROUND,
        (getDrawLayer() & BACKGROUND) && !(getDrawLayer() & FOREGROUND));
    drawButton(" FG", btns[BTN_MAPFG], getDrawLayer() & FOREGROUND,
        getDrawLayer() & FOREGROUND);

    // --- Draw file menu --
    if (menustate == MENU_FILE)
    {
        drawButton("",btns[BTN_FILE][0],10,btns[BTN_FILE][0]+50,51,false);
        drawMenuOpt("New Map", btns[BTN_NEWMAP]);
        drawMenuOpt("Save Map", btns[BTN_SAVE]);
        drawMenuOpt("Save As...", btns[BTN_SAVEAS]);
        drawMenuOpt("Load Map", btns[BTN_LOAD]);
        drawMenuOpt("Quit", btns[BTN_QUIT]);
    }
    else if (menustate == MENU_EDIT)
    {
        drawButton("",btns[BTN_EDIT][0],10,btns[BTN_EDIT][0]+83, 59, false);
        drawMenuOpt("Music", btns[BTN_MUSIC]);
        drawMenuOpt("Player Start", btns[BTN_PSTART], getMode()==PLAYER_MODE);
        drawMenuOpt("Switch Tags", btns[BTN_SWTAGS], getMode()==SWTAG_MODE);
        drawMenuOpt("Replace Tiles", btns[BTN_TILREP]);
        drawMenuOpt("Gridsnap Items", btns[BTN_GRDSNP], getGridsnap());
        drawMenuOpt("Darken BG Layer", btns[BTN_DARKBG], getDarkenBG());
    }
    else if (menustate == MENU_MATRIX)
    {
        drawButton("",btns[BTN_MATRIX][0],10,btns[BTN_MATRIX][0]+37,
            11 + 18*(int)ceil((float)numMatrices()/2.0), false);
        
        for (int i=0; i<numMatrices(); i++)
        {   int l = (int)floor((float)i/2.0);
            if (i % 2 == 0)     // EVEN MATRICES LEFT SIDE
            {   drawMenuOpt("",btns[BTN_MATRIX][0]+1, 11+l*18,
                    btns[BTN_MATRIX][0]+18, 11+l*18+17, getMatrix() == i &&
                    getMode() == MATRIX_MODE);
                drawSprite(getMatrixAt(i,1,1),btns[BTN_MATRIX][0]+2,12+l*18);
            }
            else                // ODD MATRICES RIGHT SIDE
            {   drawMenuOpt("",btns[BTN_MATRIX][0]+19, 11+l*18,
                    btns[BTN_MATRIX][0]+36, 11+l*18+17, getMatrix() == i &&
                    getMode() == MATRIX_MODE);
                drawSprite(getMatrixAt(i,1,1),btns[BTN_MATRIX][0]+20,12+l*18);
            }
        }
    }
    else if (menustate == MENU_CREATE)
    {
        drawButton("",btns[BTN_CREATE][0],10,btns[BTN_CREATE][0]+50,35,false);
        drawMenuOpt("Lift", btns[BTN_LIFT], getMode() == LIFT_MODE);
        drawMenuOpt("Locator", btns[BTN_LOCATR], getMode() == LOCATR_MODE);
        drawMenuOpt("Portal", btns[BTN_PORTAL], getMode() == PORTAL_MODE);
    }
    else if (menustate == MENU_ITEM)
    {
        drawButton("",btns[BTN_ITEM][0],10,btns[BTN_ITEM][0]+60,123,false);
        drawMenuOpt("No item", btns[BTN_NOITEM]);
        drawMenuOpt("Soda", btns[BTN_SODA],
            getMode() == ITEM_MODE && getItem() == SODA);
        drawMenuOpt("Gum", btns[BTN_GUM],
            getMode() == ITEM_MODE && getItem() == GUM);
        drawMenuOpt("Chocolate", btns[BTN_CHOCO],
            getMode() == ITEM_MODE && getItem() == CHOCOLATE);
        drawMenuOpt("Jawbreaker", btns[BTN_CANDY],
            getMode() == ITEM_MODE && getItem() == CANDY);
        drawMenuOpt("Donut", btns[BTN_DONUT],
            getMode() == ITEM_MODE && getItem() == DONUT);
        drawMenuOpt("Icecream", btns[BTN_ICECRM],
            getMode() == ITEM_MODE && getItem() == ICECREAM);
        drawMenuOpt("Flask", btns[BTN_FLASK],
            getMode() == ITEM_MODE && getItem() == FLASK);
        drawMenuOpt("Blaster", btns[BTN_GUN],
            getMode() == ITEM_MODE && getItem() == GUN);
        drawMenuOpt("Drop", btns[BTN_DROP],
            getMode() == ITEM_MODE && getItem() == DROP);
        drawMenuOpt("Red Gem", btns[BTN_REDKEY],
            getMode() == ITEM_MODE && getItem() == REDKEY);
        drawMenuOpt("Blue Gem", btns[BTN_BLUKEY],
            getMode() == ITEM_MODE && getItem() == BLUKEY);
        drawMenuOpt("Green Gem", btns[BTN_GRNKEY],
            getMode() == ITEM_MODE && getItem() == GRNKEY);
        drawMenuOpt("Yellow Gem", btns[BTN_YELKEY],
            getMode() == ITEM_MODE && getItem() == YELKEY);
    }
    else if (menustate == MENU_ENEMY)
    {
        drawButton("",btns[BTN_ENEMY][0],10,btns[BTN_ENEMY][0]+75,163,false);
        drawMenuOpt("No enemy", btns[BTN_NOENMY]);
        drawMenuOpt("Slug", btns[BTN_SLUG],
            getMode() == ENEMY_MODE && getEnemy() == SLUG);
        drawMenuOpt("Bounder", btns[BTN_BALL],
            getMode() == ENEMY_MODE && getEnemy() == BOUNDER);
        drawMenuOpt("Mushroom", btns[BTN_MUSH],
            getMode() == ENEMY_MODE && getEnemy() == MUSHROOM);
        drawMenuOpt("Lick", btns[BTN_LICK],
            getMode() == ENEMY_MODE && getEnemy() == LICK);
        drawMenuOpt("Arachnut", btns[BTN_ARACH],
            getMode() == ENEMY_MODE && getEnemy() == ARACHNUT);
        drawMenuOpt("Worm", btns[BTN_WORM],
            getMode() == ENEMY_MODE && getEnemy() == WORM);
        drawMenuOpt("Snake", btns[BTN_SNAKE],
            getMode() == ENEMY_MODE && getEnemy() == SNAKE);
        drawMenuOpt("Ancient", btns[BTN_OLDMAN],
            getMode() == ENEMY_MODE && getEnemy() == ANCIENT);
        drawMenuOpt("Spear (Up)", btns[BTN_SPEARU],
            getMode() == ENEMY_MODE && getEnemy() == SPEARUP);
        drawMenuOpt("Spear (Down)", btns[BTN_SPEARD],
            getMode() == ENEMY_MODE && getEnemy() == SPEARDOWN);
        drawMenuOpt("Spear (Left)", btns[BTN_SPEARL],
            getMode() == ENEMY_MODE && getEnemy() == SPEARLEFT);
        drawMenuOpt("Spear (Right)", btns[BTN_SPEARR],
            getMode() == ENEMY_MODE && getEnemy() == SPEARRIGHT);
        drawMenuOpt("Shooter (Up)", btns[BTN_SHOTRU],
            getMode() == ENEMY_MODE && getEnemy() == SHOOTERUP);
        drawMenuOpt("Shooter (Down)", btns[BTN_SHOTRD],
            getMode() == ENEMY_MODE && getEnemy() == SHOOTERDOWN);
        drawMenuOpt("Shooter (Left)", btns[BTN_SHOTRL],
            getMode() == ENEMY_MODE && getEnemy() == SHOOTERLEFT);
        drawMenuOpt("Shooter (Right)", btns[BTN_SHOTRR],
            getMode() == ENEMY_MODE && getEnemy() == SHOOTERRIGHT);
        drawMenuOpt("Mimrock", btns[BTN_MROCK],
            getMode() == ENEMY_MODE && getEnemy() == MIMROCK);
        drawMenuOpt("Cloud", btns[BTN_CLOUD],
            getMode() == ENEMY_MODE && getEnemy() == CLOUD);
    }
}

/* ------------------------------------------------------------------------- *
 * drawOnjectBar()                                                           *
 *                                                                           *
 * Draws the dragable object bar wherever it is on the screen.               *
 * ------------------------------------------------------------------------- */
void Menu::drawObjectBar()
{
    drawButton("", objbar.x, objbar.y, objbar.x+100, objbar.y+20, false);
    switch (getMode())
    {
        case TILE_MODE:
        drawBox(objbar.x+1, objbar.y+1, objbar.x+18, objbar.y+18, 16, EMPTY);
        drawSprite(getDrawTile(), objbar.x+2, objbar.y+2, 16);
        if (getDrawTile() == 0)
        {   drawText("Tile 0", objbar.x+20, objbar.y+2, 16);
            drawText("Erase", objbar.x+20, objbar.y+12, 16);
        }
        else
        {   string description = tileDesc(getDrawTile());
            if (tileDesc2(getDrawTile()) != "")
            {   description += " (" + tileDesc2(getDrawTile()) + ")"; }
            drawText("TILE "+toStr(getDrawTile()),objbar.x+20,objbar.y+2,16);
            drawText(description, objbar.x+20, objbar.y+12, 16);
        } break;
        case MATRIX_MODE:
        drawBox(objbar.x+1, objbar.y+1, objbar.x+18, objbar.y+18, 16, EMPTY);
        drawSprite(getMatrixAt(getMatrix(),1,1), objbar.x+2, objbar.y+2, 16);
        drawText("Matrix "+toStr(getMatrix()), objbar.x+20, objbar.y+2, 16);
        drawText("Offset:", objbar.x+20, objbar.y+12, 16);
        drawButton(toStr(getMatrixOffsetX(getMatrix())), objbar.x+54,
            objbar.y+10,objbar.x+74,objbar.y+19, false);
        drawText(",", objbar.x+76, objbar.y+12, 16);
        drawButton(toStr(getMatrixOffsetY(getMatrix())), objbar.x+79,
            objbar.y+10,objbar.x+99,objbar.y+19, false);
        break;
        case ITEM_MODE:
        drawBox(objbar.x+1, objbar.y+1, objbar.x+18, objbar.y+18, 16, EMPTY);
        drawSprite(Item::getIcon(getItem()), objbar.x+2, objbar.y+2);
        drawText("Item", objbar.x+20, objbar.y+2, 16);
        drawText(Item::getName(getItem()), objbar.x+20, objbar.y+12, 16);
        break;
        case ENEMY_MODE:
        drawBox(objbar.x+1, objbar.y+1, objbar.x+18, objbar.y+18, 16, EMPTY);
        drawSprite(Enemy::getIcon(getEnemy(), getDir()),
            objbar.x+2, objbar.y+2, 16);
        drawText("Enemy", objbar.x+20, objbar.y+2, 16);
        drawText(Enemy::getName(getEnemy()), objbar.x+20, objbar.y+12, 16);
        drawButton(" <", objbar.x+66, objbar.y+1, objbar.x+82, objbar.y+11,
            getDir() == LEFT);
        drawButton(" >", objbar.x+83, objbar.y+1, objbar.x+99, objbar.y+11,
            getDir() == RIGHT);
        break;
        case DEL_MODE:
        drawText("Delete Mode", objbar.x+2, objbar.y+2, 16); break;
        case PLAYER_MODE:
        drawText("Player Start", objbar.x+2, objbar.y+2, 16);
        drawButton(" <", objbar.x+66, objbar.y+1, objbar.x+82, objbar.y+11,
            getDir() == LEFT);
        drawButton(" >", objbar.x+83, objbar.y+1, objbar.x+99, objbar.y+11,
            getDir() == RIGHT);
        break;
        case SWTAG_MODE:
        drawText("Switch Tags", objbar.x+2, objbar.y+2, 16);
        break;
        case PORTAL_MODE:
        drawText("Place Portal", objbar.x+2, objbar.y+2, 16);
        if (getPortal() == PLACE_ENTER)
        {   drawText("Place Entrance", objbar.x+2, objbar.y+12, 16); }
        else if (getPortal() == PLACE_EXIT)
        {   drawText("Place Exit", objbar.x+2, objbar.y+12, 16); }
        break;
        case LOCATR_MODE:
        drawText("Place Locator", objbar.x+2, objbar.y+2, 16);
        drawText("Track: "+toStr(getTrack()), objbar.x+2, objbar.y+12, 16);
        drawButton(" -", objbar.x+66, objbar.y+9, objbar.x+82, objbar.y+19,
            false);
        drawButton(" +", objbar.x+83, objbar.y+9, objbar.x+99, objbar.y+19,
            false);
        break;
        case LIFT_MODE:
        drawText("Place Lift", objbar.x+2, objbar.y+2, 16);
        drawText("Track: "+toStr(getTrack()), objbar.x+2, objbar.y+12, 16);
        drawButton(" -", objbar.x+66, objbar.y+9, objbar.x+82, objbar.y+19,
            false);
        drawButton(" +", objbar.x+83, objbar.y+9, objbar.x+99, objbar.y+19,
            false);
        break;
    }
}

/* ------------------------------------------------------------------------- *
 * drawButton(string text, int x1, int y1, int x2, int y2, bool on)          *
 *   text: label on the button                                               *
 *   x1,y1: top left corner                                                  *
 *   x2,y2: bottom left corner                                               *
 *   on: is the button pressed                                               *
 *                                                                           *
 * Draws a menu button in its current state.                                 *
 * ------------------------------------------------------------------------- */
void Menu::drawButton(string text, int x1, int y1, int x2, int y2, bool on,
    bool selected)
{
    if (selected)
    {   drawBox(x1, y1, x2, y2, 23, FILLED);
        drawLine(x1, y1, x2, y1, 26);       // top
        drawLine(x1, y1, x1, y2, 26);       // left
        drawLine(x1, y2, x2, y2, 20);       // bottom
        drawLine(x2, y2, x2, y1, 20);       // right
        drawText(text, x1+2, y1+2, 32);
    }
    else if (on)
    {   drawBox(x1, y1, x2, y2, 23, FILLED);
        drawLine(x1, y1, x2, y1, 26);       // top
        drawLine(x1, y1, x1, y2, 26);       // left
        drawLine(x1, y2, x2, y2, 20);       // bottom
        drawLine(x2, y2, x2, y1, 20);       // right
        drawText(text, x1+2, y1+2, 16);
    }
    else
    {   if (text != "" && mouseIn(x1, y1, x2, y2))
        {   drawBox(x1, y1, x2, y2, 20, FILLED);
            drawLine(x1, y1, x2, y1, 20);       // top
            drawLine(x1, y1, x1, y2, 20);       // left
            drawLine(x1, y2, x2, y2, 26);       // bottom
            drawLine(x2, y2, x2, y1, 26);       // right
            drawText(text, x1+2, y1+2, 14);
        }
        else
        {   drawBox(x1, y1, x2, y2, 22, FILLED);
            drawLine(x1, y1, x2, y1, 20);       // top
            drawLine(x1, y1, x1, y2, 20);       // left
            drawLine(x1, y2, x2, y2, 26);       // bottom
            drawLine(x2, y2, x2, y1, 26);       // right
            drawText(text, x1+2, y1+2, 14);
        }
    }
}

void Menu::drawButton(string text, int corners[], bool on,
    bool selected)
{
    int x1 = corners[0], y1 = corners[1];
    int x2 = corners[2], y2 = corners[3];
    if (selected)
    {   drawBox(x1, y1, x2, y2, 23, FILLED);
        drawLine(x1, y1, x2, y1, 26);       // top
        drawLine(x1, y1, x1, y2, 26);       // left
        drawLine(x1, y2, x2, y2, 20);       // bottom
        drawLine(x2, y2, x2, y1, 20);       // right
        drawText(text, x1+2, y1+2, 32);
    }
    else if (on)
    {   drawBox(x1, y1, x2, y2, 23, FILLED);
        drawLine(x1, y1, x2, y1, 26);       // top
        drawLine(x1, y1, x1, y2, 26);       // left
        drawLine(x1, y2, x2, y2, 20);       // bottom
        drawLine(x2, y2, x2, y1, 20);       // right
        drawText(text, x1+2, y1+2, 16);
    }
    else
    {   if (text != "" && mouseIn(x1, y1, x2, y2))
        {   drawBox(x1, y1, x2, y2, 20, FILLED);
            drawLine(x1, y1, x2, y1, 20);       // top
            drawLine(x1, y1, x1, y2, 20);       // left
            drawLine(x1, y2, x2, y2, 26);       // bottom
            drawLine(x2, y2, x2, y1, 26);       // right
            drawText(text, x1+2, y1+2, 14);
        }
        else
        {   drawBox(x1, y1, x2, y2, 22, FILLED);
            drawLine(x1, y1, x2, y1, 20);       // top
            drawLine(x1, y1, x1, y2, 20);       // left
            drawLine(x1, y2, x2, y2, 26);       // bottom
            drawLine(x2, y2, x2, y1, 26);       // right
            drawText(text, x1+2, y1+2, 14);
        }
    }
}

/* ------------------------------------------------------------------------- *
 * void drawMenuOpt(string text, int corners[],)                             *
 *   text: label of menu option                                              *
 *   corners: array of the corners of the option                             *
 *                                                                           *
 * Draws an option of a pulldown menu like the file, edit and create menus.  *
 * The menu option will be automatically highlighted blue if the mouse is    *
 * within the corners rectangle.                                             *
 * ------------------------------------------------------------------------- */
void Menu::drawMenuOpt(string text, int corners[], bool selected)
{
    if (mouseIn(corners) && selected)
    {   drawBox(corners[0], corners[1], corners[2], corners[3], 167, FILLED);
        drawText(text, corners[0]+1, corners[1]+1, 16); }
    else if (selected)
    {   drawBox(corners[0], corners[1], corners[2], corners[3], 36, FILLED);
        drawText(text, corners[0]+1, corners[1]+1, 16); }
    else if (mouseIn(corners))
    {   drawBox(corners[0], corners[1], corners[2], corners[3], 148, FILLED);
        drawText(text, corners[0]+1, corners[1]+1, 16); }
    else
    {   drawText(text, corners[0]+1, corners[1]+1, 14); }
}

void Menu::drawMenuOpt(string text, int x1, int y1, int x2, int y2,
                       bool selected)
{
    if (mouseIn(x1, y1, x2, y2) && selected)
    {   drawBox(x1, y1, x2, y2, 167, FILLED);
        drawText(text, x1+1, y1+1, 16); }
    else if (selected)
    {   drawBox(x1, y1, x2, y2, 36, FILLED);
        drawText(text, x1+1, y1+1, 16); }
    else if (mouseIn(x1, y1, x2, y2))
    {   drawBox(x1, y1, x2, y2, 148, FILLED);
        drawText(text, x1+1, y1+1, 16); }
    else
    {   drawText(text, x1+1, y1+1, 14); }
}

/* ------------------------------------------------------------------------- *
 * void toggleMouseCoords()                                                  *
 *                                                                           *
 * Toggles the units of the mouse coordinates on the menu bar.               *
 * ------------------------------------------------------------------------- */
void Menu::toggleMouseCoordsType()
{   if (mouseCoordsType == TILE_COORDS) mouseCoordsType = PIXEL_COORDS;
    else mouseCoordsType = TILE_COORDS;
}

/* ------------------------------------------------------------------------- *
 * int chooseTile(int curtile)                                               *
 *   curtile: the current chosen tile which will be highlighted in red       *
 *                                                                           *
 * Displays the tile selection box and allows the user to choose a tile. The *
 * tile number chosen is returned or -1 if the prompt is cancelled.          *
 * ------------------------------------------------------------------------- */
int Menu::chooseTile(int curtile)
{
    //int tileChooserRow = max(0, ((curtile - curtile%18) / 18) - 6);
    long scrollmark = timerTicks(HIRESTIMER);       // for scrolling delay
    do
    {   trackMouse();
        if (timerTicked(FRAMETIMER))
        {   // --------------------- DRAW PROMPT ---------------------
            drawButton("", 13, 18, 607, 27, false);         // title bar
            drawText("Tile List", 13+2, 18+2, 14);          // title bar text
            drawButton(" X", 608, 18, 627, 27, false);      // close button
            drawButton("", 13, 28, 627, 471, false);        // gray background

            for (int row=0; row<13; row++)
            {   for (int col=0; col<18; col++)
                {   // corners of clickable box around tile
                    int x1 = 15+col*34-1,  y1 = 30+row*34-1;
                    int x2 = 15+col*34+32, y2 = 30+row*34+32;
                    drawButton("", x1, y1, x2, y2, false);
                    // -- Draw a blue or purple border on mouseOver --
                    if (mouseIn(x1, y1, x2, y2))
                    {   if (curtile == (tileChooserRow+row)*18+col+1)
                        // This tile mouseOver and selected
                        {   drawBox(x1, y1, x2, y2, 167, FILLED); }
                        else
                        // This tile mouseOver only
                        {   drawBox(x1, y1, x2, y2, 148, FILLED); }
                    }
                    else if (curtile == (tileChooserRow+row)*18+col+1)
                    // This tile selected only
                    {   drawBox(x1, y1, x2, y2, 32, FILLED); }
                    
                    // -- Draw the tile --
                    drawSprite((tileChooserRow+row)*18+col+1, x1+1, y1+1, 32);
                }
            }
            blitToScreen();
        }   // -------------------------------------------------------

        // ----------------------- MOUSE INPUT -----------------------
        if (mouseLB())
        {   
            if (mouseIn(14, 29, 626, 470))
            {   for(int r=0; r<13; r++)
                {   for (int c=0; c<18; c++)
                    {   // Check if mouse clicked on tile at (c,r)
                        int x1 = 15 + c * 34 - 1,  y1 = 30 + r * 34 - 1;
                        int x2 = 15 + c * 34 + 32, y2 = 30 + r * 34 + 32;
                        if (mouseIn(x1, y1, x2, y2))
                        {   waitForLB();
                            if (mouseIn(x1, y1, x2, y2) &&
                                (tileChooserRow+r)*18+c+1 < getNumTiles())
                            {   return (tileChooserRow+r)*18+c+1; }
                            else { r=99; c=99; }    // stop checking mouse pos,
                        }                           // user clicked and then
                    }                               // moved the mouse away
                }
            }
            else if (mouseIn(608, 18, 627, 27))
            {   waitForLB(); if (mouseIn(608, 18, 627, 27)) return -1; }
        }   
        if (mouseRB())
        {   if (mouseIn(14, 29, 626, 470))
            {   for(int r=0; r<13; r++)
                {   for (int c=0; c<18; c++)
                    {   // Check if mouse clicked on tile at (c,r)
                        int x1 = 15 + c * 34 - 1,  y1 = 30 + r * 34 - 1;
                        int x2 = 15 + c * 34 + 32, y2 = 30 + r * 34 + 32;
                        if (mouseIn(x1, y1, x2, y2))
                        {   //waitForRB();
                            if (mouseIn(x1, y1, x2, y2) &&
                                (tileChooserRow+r)*18+c+1 < getNumTiles())
                            {   // Keep the popup box from going outside the
                                // tilechooser window
                                int bx1 = mouseX() + 10, bx2 = mouseX() + 95;
                                int by1 = mouseY() - 2, by2 = mouseY() + 17;
                                if (bx1 < 13) { bx2+=13-bx1; bx1+=13-bx1; }
                                if (bx2 > 627) { bx1+=627-bx2; bx2+=627-bx2; }
                                if (by1 < 28) { by2+=28-by1; by1+=28-by1; }
                                if (by2 > 471) { by1+=471-by2; by2+=471-by2; }
                                drawBox(bx1, by1, bx2, by2, 65, FILLED);
                                drawBox(bx1, by1, bx2, by2, 14, EMPTY);
                                int tile = (tileChooserRow+r)*18+c+1;
                                drawText("TILE "+toStr(tile), bx1+3, by1+3,14);
                                string description = tileDesc(tile);
                                if (tileDesc2(tile) != "")
                                {   description += " ("+tileDesc2(tile)+")"; }
                                drawText(description, bx1 + 3, by1 + 11, 14);
                                blitToScreen();
                                waitForRB();
                            }
                            else { r=99; c=99; }    // stop checking mouse pos,
                        }                           // user clicked and then
                    }                               // moved the mouse away
                }
            }
        }
        // ----------------------------------------------------------

        // --------------------- KEYBOARD INPUT ---------------------
        if(key[KEY_ESC])
        {   while(key[KEY_ESC]); return -1; }
        else if (key[KEY_UP] && timerTicks(HIRESTIMER) >= scrollmark)
        {   if (tileChooserRow > 0) tileChooserRow--;
            scrollmark = timerTicks(HIRESTIMER) + 15; }
        else if (key[KEY_DOWN] && timerTicks(HIRESTIMER) >= scrollmark)
        {   if ((tileChooserRow+5)*18+18 < getNumTiles()) tileChooserRow++;
            scrollmark = timerTicks(HIRESTIMER) + 15; }
        else if (key[KEY_PGUP] && timerTicks(HIRESTIMER) >= scrollmark)
        {   tileChooserRow -= 13;
            if (tileChooserRow < 0) tileChooserRow = 0;
            scrollmark = timerTicks(HIRESTIMER) + 40; }
        else if (key[KEY_PGDN] && timerTicks(HIRESTIMER) >= scrollmark)
        {   tileChooserRow += 13;
            if ((tileChooserRow+5)*18+18 >= getNumTiles())
            {   tileChooserRow = (getNumTiles() / 18) - 5; }
            scrollmark = timerTicks(HIRESTIMER) + 40; }
        // ----------------------------------------------------------

        usleep(1000);
    } while (!getQuitGame());
    return -1;
}

/* ------------------------------------------------------------------------- *
 * void chooseMusic()                                                        *
 *                                                                           *
 * Displays the music selection box and allows the user to choose a          *
 * background mp3 track.                                                     *
 * ------------------------------------------------------------------------- */
void Menu::chooseMusic()
{
    bool exit = false;
    bool playing = false;           // music playing
    
    string status = "";
    string musfile = mapGetMusic();
    if (!Audio::loadMusic(musfile)) { status = "File not found"; }
    else { status = "File loaded"; }
    
    do
    {   trackMouse();
        Audio::tickMusic();
        if (timerTicked(FRAMETIMER))
        {   // --------------------- DRAW PROMPT ---------------------
            drawButton("", 220, 190, 400, 199, false);      // title bar
            drawText("Background Music", 222, 192, 14);     // title label
            drawButton(" X", 401, 190, 420, 199, false);    // close button
            drawButton("", 220, 200, 420, 290, false);      // background

            drawBox(230, 210, 350, 220, 14, FILLED);
            drawBox(230, 210, 350, 220, 16, EMPTY);
            drawText(musfile, 232, 212, 70);
            drawButton("Change", 360, 210, 405, 220, false);

            drawText("Status: "+status, 230, 225, 14);      // status text

            drawButton(" ", 230, 260, 260, 280, playing);   // play/stop
            drawText("Play", 236, 267, 14+2*playing);       // buttons
            drawButton(" ", 270, 260, 300, 280, !playing);  //
            drawText("Stop", 276, 267, 14+2*(!playing));    //

            drawButton(" OK", 380, 270, 405, 280,
                status == "File not found" && musfile != "");

            blitToScreen();
        }   // -------------------------------------------------------
        
        if (mouseLB())
        {   // --------------------- MOUSE INPUT ---------------------
            if (mouseIn(401, 190, 420, 199))
            {   waitForLB();
                if (mouseIn(401, 190, 420, 199)) { exit = true; }
            }
            else if (mouseIn(360, 210, 409, 220))
            {   waitForLB();
                if (mouseIn(360, 210, 409, 220))
                {   Audio::stopMusic(); playing = false;
                    drawButton("Change", 360, 210, 405, 220, true);
                    blitToScreen();
                    drawBox(231, 211, 349, 219, 14, FILLED);
                    musfile = getTextInput(232, 212, 70, 12, 118, musfile);
                    if (!Audio::loadMusic(musfile))
                    {   status = "File not found"; }
                    else { status = "File loaded"; }
                }
            }
            else if (mouseIn(230, 260, 260, 280) && !playing && 
                     status == "File loaded")
            {   waitForLB();
                if (mouseIn(230, 260, 260, 280))
                {   Audio::playMusic(); playing = true; }
            }
            else if (mouseIn(270, 260, 300, 280) && playing)
            {   waitForLB();
                if (mouseIn(270, 260, 300, 280))
                {   Audio::stopMusic(); playing = false;
                    Audio::loadMusic(musfile);
                }
            }
            else if (mouseIn(380, 270, 405, 280) &&
                (status == "File loaded" || musfile == ""))
            {   waitForLB();
                if (mouseIn(380, 270, 405, 280))
                {   mapSetMusic(musfile); exit = true; }
            }   
        }   // -------------------------------------------------------

        usleep(1000);
    } while (!exit && !getQuitGame());
    
    Audio::stopMusic();
}

/* ------------------------------------------------------------------------- *
 * void promptSwitchTag()                                                    *
 *   x,y: tile location of tag                                               *
 *                                                                           *
 * Edit the value of a tile on the switch tag layer.                         *
 * ------------------------------------------------------------------------- */
void Menu::promptSwitchTag(int x, int y)
{
    // draw switch tag prompt and get value
    drawButton("Switch Tag "+toStr(x)+","+toStr(y)+":", 268, 225, 372,
        246, false);
    drawBox(270, 235, 370, 244, 14, FILLED);
    drawBox(270, 235, 370, 244, 16, EMPTY);
    blitToScreen();
    string s = getTextInput(272, 237, 70, 6, 90, toStr(getSwitchTag(x,y)));
    if (s != "") setSwitchTag(x, y, toInt(s));
}

/* ------------------------------------------------------------------------- *
 * void promptCreateLocator(int &delay, int &locksw)                         *
 *   delay : time for lift to stop at locator                                *
 *   locksw: switch which locks lift at this locator                         *
 *                                                                           *
 * Get the parameters from the user for a new locator.                       *
 * ------------------------------------------------------------------------- */
void Menu::promptCreateLocator(int &delay, int &locksw)
{
    // draw locator tag prompt and get values
    drawButton("", 275, 190, 360, 199, false);      // title bar
    drawText("New Locator", 277, 192, 14);          // title label
    drawButton("", 275, 200, 360, 230, false);      // background

    drawText("Delay:", 277, 204, 14);
    drawBox(313, 202, 350, 212, 14, FILLED);
    drawBox(313, 202, 350, 212, 16, EMPTY);
    drawText("Switch:", 277, 218, 14);
    drawBox(313, 216, 350, 226, 14, FILLED);
    drawBox(313, 216, 350, 226, 16, EMPTY);
    blitToScreen();
    waitForLB();

    string s;
    s = getTextInput(315, 204, 70, 4, 40, "");
    if (s != "") delay = toInt(s); else { delay = -1; locksw = -1; return; }
    drawBox(314, 203, 349, 211, 14, FILLED);
    drawText(toStr(delay), 315, 204, 70); blitToScreen();
    s = getTextInput(315, 218, 70, 4, 40, "");
    if (s != "") locksw = toInt(s); else { delay = -1; locksw = -1; return; }
}

/* ------------------------------------------------------------------------- *
 * void promptNewMap()                                                       *
 *                                                                           *
 * Displays the prompt for creating a new map.                               *
 * ------------------------------------------------------------------------- */
void Menu::promptNewMap()
{
    const int MIN_WIDTH =  20, MIN_HEIGHT =  13;
    const int MAX_WIDTH = 512, MAX_HEIGHT = 512;
    int width = MIN_WIDTH, height = MIN_HEIGHT;
    
    do
    {   trackMouse();
        if (timerTicked(FRAMETIMER))
        {   // --------------------- DRAW PROMPT ---------------------
            drawButton("", 265, 190, 355, 199, false);      // title bar
            drawText("New Map", 267, 192, 14);              // title label
            drawButton(" X", 356, 190, 375, 199, false);    // close button
            drawButton("", 265, 200, 375, 255, false);      // background

            drawBox(270, 205, 330, 215, 14, FILLED);
            drawBox(270, 205, 330, 215, 16, EMPTY);
            drawText(toStr(width), 272, 207, 70);
            drawButton("Width", 335, 205, 370, 215, false);
            
            drawBox(270, 220, 330, 230, 14, FILLED);
            drawBox(270, 220, 330, 230, 16, EMPTY);
            drawText(toStr(height), 272, 222, 70);
            drawButton("Height", 335, 220, 370, 230, false);
            
            drawButton(" OK", 305, 240, 330, 250, false);
            drawButton("Cancel", 335, 240, 370, 250, false);
            
            blitToScreen();
        }   // -------------------------------------------------------
        
        if (mouseLB())
        {   // --------------------- MOUSE INPUT ---------------------
            // X button in top right
            if (mouseIn(356, 190, 375, 199))
            {   drawButton(" X", 356, 190, 375, 199, true);
                waitForLB();
                if (mouseIn(356, 190, 375, 199)) return; }
            // Cancel button
            else if (mouseIn(335, 240, 370, 250))
            {   drawButton("Cancel", 335, 240, 370, 250, true);
                waitForLB();
                if (mouseIn(335, 240, 370, 250)) return; }
            // Width button
            else if (mouseIn(335, 205, 370, 215))
            {   drawButton("Width", 335, 205, 370, 215, true);
                waitForLB();
                if (mouseIn(335, 205, 370, 215))
                {   drawBox(271, 206, 329, 214, 14, FILLED);
                    string s = getTextInput(272, 207, 70, 4, 50, toStr(width));
                    if (s != "")
                    {   width = max(toInt(s), MIN_WIDTH);
                        width = min(width, MAX_WIDTH); }
                }                            
            }
            // Height button
            else if (mouseIn(335, 220, 370, 230))
            {   drawButton("Height", 335, 220, 370, 230, true);
                waitForLB();
                if (mouseIn(335, 220, 370, 230))
                {   drawBox(271, 221, 329, 229, 14, FILLED);
                    string s = getTextInput(272, 222, 70, 4, 50,
                               toStr(height));
                    if (s != "")
                    {   height = max(toInt(s), MIN_HEIGHT);
                        height = min(height, MAX_HEIGHT); }
                }
            }
            // OK button
            else if (mouseIn(305, 240, 330, 250))
            {   drawButton(" OK", 305, 240, 330, 250, true);
                waitForLB();
                if (mouseIn(305, 240, 330, 250))
                {   mapNew(width, height);
                    currentMap = "new.map"; return; }
            }
        }   // -------------------------------------------------------

        usleep(1000);
    } while (!getQuitGame());
}

/* ------------------------------------------------------------------------- *
 * void promptMatrixOffsetX()                                                *
 * void promptMatrixOffsetY()                                                *
 *                                                                           *
 * Each of the prompts for changing a matrix's offset x or y.                *
 * ------------------------------------------------------------------------- */
void Menu::promptMatrixOffsetX()
{
    // draw offset button as pressed
    drawButton(toStr(getMatrixOffsetX(getMatrix())), objbar.x+54, objbar.y+10,
        objbar.x+74, objbar.y+19, true);
    
    // draw offset prompt and get value
    drawButton("Matrix "+toStr(getMatrix())+" Offset X:", 273, 225, 367,
        246, false);
    drawBox(275, 235, 365, 244, 14, FILLED);
    drawBox(275, 235, 365, 244, 16, EMPTY);
    blitToScreen();
    string offx = getTextInput(277, 237, 70, 3, 90,
        toStr(getMatrixOffsetX(getMatrix())));
    setMatrixOffset(getMatrix(), toInt(offx), getMatrixOffsetY(getMatrix()));
}

void Menu::promptMatrixOffsetY()
{
    // draw offset button as pressed
    drawButton(toStr(getMatrixOffsetY(getMatrix())), objbar.x+79,
        objbar.y+10,objbar.x+99,objbar.y+19, true);
    
    // draw offset prompt and get value
    drawButton("Matrix "+toStr(getMatrix())+" Offset Y:", 273, 225, 367,
        246, false);
    drawBox(275, 235, 365, 244, 14, FILLED);
    drawBox(275, 235, 365, 244, 16, EMPTY);
    blitToScreen();
    string offy = getTextInput(277, 237, 70, 3, 90,
        toStr(getMatrixOffsetY(getMatrix())));
    setMatrixOffset(getMatrix(), getMatrixOffsetX(getMatrix()), toInt(offy));

}

/* ------------------------------------------------------------------------- *
 * void promptTileReplace()                                                  *
 *                                                                           *
 * Handles a prompt for globally replacing one tile in the map with another. *
 * ------------------------------------------------------------------------- */
void Menu::promptTileReplace()
{
    int tile1 = 0, tile2 = 0;
    bool exit = false;
    
    do
    {   trackMouse();
        if (timerTicked(FRAMETIMER))
        {   // --------------------- DRAW PROMPT ---------------------
            drawButton("", 220, 190, 400, 199, false);      // title bar
            drawText("Tile Replacement", 222, 192, 14);     // title label
            drawButton(" X", 401, 190, 420, 199, false);    // close button
            drawButton("", 220, 200, 420, 270, false);      // background

            drawText("Replace all tiles", 255, 210, 14);
            drawBox(335, 208, 364, 218, 14, FILLED);
            drawBox(335, 208, 364, 218, 16, EMPTY);
            drawText(toStr(tile1), 337, 210, 70);
            drawButton("Change", 370, 208, 410, 218, false);
            
            drawText("with tile", 291, 225, 14);
            drawBox(335, 223, 364, 233, 14, FILLED);
            drawBox(335, 223, 364, 233, 16, EMPTY);
            drawText(toStr(tile2), 337, 225, 70);
            drawButton("Change", 370, 223, 410, 233, false);
            
            drawBox(292, 245, 309, 262, 16, EMPTY);
            drawSprite(tile1, 293, 246, 16);
            drawText("->", 317, 251, 14);
            drawBox(331, 245, 348, 262, 16, EMPTY);
            drawSprite(tile2, 332, 246, 16);
            
            drawButton(" OK", 385, 252, 410, 262, tile1 == tile2);

            blitToScreen();
        }   // -------------------------------------------------------
        
        if (mouseLB())
        {   // --------------------- MOUSE INPUT ---------------------
            if (mouseIn(401, 190, 420, 199))
            {   waitForLB();
                if (mouseIn(401, 190, 420, 199)) { exit = true; } }
            else if (mouseIn(385, 252, 410, 262) && tile1 != tile2)
            {   waitForLB();
                if (mouseIn(385, 252, 410, 262))
                { mapReplaceTile(tile1, tile2); exit = true; }
            }
            else if (mouseIn(370, 208, 410, 218))
            {   waitForLB();
                if (mouseIn(370, 208, 410, 218))
                {   drawButton("Change", 370, 208, 410, 218, true);
                    blitToScreen();
                    drawBox(336, 209, 363, 217, 14, FILLED);
                    tile1 = toInt(getTextInput(337, 210, 70, 4, 25,
                            toStr(tile1)));
                }
            }
            else if (mouseIn(370, 223, 410, 233))
            {   waitForLB();
                if (mouseIn(370, 223, 410, 233))
                {   drawButton("Change", 370, 223, 410, 233, true);
                    blitToScreen();
                    drawBox(336, 224, 363, 232, 14, FILLED);
                    tile2 = toInt(getTextInput(337, 225, 70, 4, 25,
                            toStr(tile2)));
                }
            }
            else if (mouseIn(293, 246, 308, 261))
            {   waitForLB();
                if (mouseIn(293, 246, 308, 261))
                {   // Save background and restore after tile choose is gone
                    BITMAP* buffer = create_bitmap(640, 480);
                    grabBitmap(buffer, 0, 0, 0, 0, 640, 480);
                    int t = chooseTile(tile1); if (t >= 0) tile1 = t;
                    drawBitmap(buffer, 0, 0);
                    destroy_bitmap(buffer);
                }
            }
            else if (mouseIn(332, 246, 347, 261))
            {   waitForLB();
                if (mouseIn(332, 246, 347, 261))
                {   // Save background and restore after tile choose is gone
                    BITMAP* buffer = create_bitmap(640, 480);
                    grabBitmap(buffer, 0, 0, 0, 0, 640, 480);
                    int t = chooseTile(tile2); if (t >= 0) tile2 = t;
                    drawBitmap(buffer, 0, 0);
                    destroy_bitmap(buffer);
                }
            }
        }   // -------------------------------------------------------

        usleep(1000);
    } while (!exit && !getQuitGame());
}

/* ------------------------------------------------------------------------- *
 * string getTextInput(int x, int y, int c, int pixwidth)                    *
 *   x,y: location of top left of text                                       *
 *   c  : color of text                                                      *
 *   maxchars: the maximum characters to accept                              *
 *   pixwidth: max width of input in pixels                                  *
 *                                                                           *
 * Gets a string input from the user.                                        *
 * ------------------------------------------------------------------------- */
string Menu::getTextInput(int x, int y, int c, int maxchars, int pixwidth,
    string input)
{
    // --- save region behind the text area ---
    BITMAP* buffer = create_bitmap(pixwidth, 8);
    grabBitmap(buffer, x, y, 0, 0, pixwidth, 8);

    clear_keybuf();
    
    do
    {
        if (timerTicked(FRAMETIMER))
        {   drawBitmap(buffer, x, y);   // Draw text background
            drawText(input, x, y, c);   // Draw input text
            
            // Draw vertical line cursor (if theres room)
            if (getTextLen(input)+3 < pixwidth)
            {   drawBox(x+getTextLen(input)+2, y, x+getTextLen(input)+2,
                    y+6, c, EMPTY); }
            blitToScreen();
        }

        if (keypressed())
        {   int key = (readkey() & 0xff);
            if ((key & 0xff) == 8)
            {   if (input.length() > 0)
                {   input = input.substr(0, input.length()-1); }
            }
            else if ((key & 0xff) == 13) { return input; }
            else if ((key & 0xff) == 27) { return ""; }
            else if ((key & 0xff) > 31 && (key & 0xff) < 127)
            {   if (input.length() < maxchars &&
                    getTextLen(input+(char)(key & 0xff)) < pixwidth)
                {   input += (char)(key & 0xff); }
            }
        }

        usleep(1000);
    }
    while (true);
    
    destroy_bitmap(buffer);
    return input;
}

/* ------------------------------------------------------------------------- *
 * string toStr(int n)                                                       *
 *   n: integer to convert to string                                         *
 *                                                                           *
 * Returns a string representation of an integer 'n'.                        *
 * ------------------------------------------------------------------------- */
string Menu::toStr(int n)
{
    char buffer[11];
    return string(itoa(n, buffer, 10));
}

/* ------------------------------------------------------------------------- *
 * int toInt(int n)                                                          *
 *   n: string of number to convert                                          *
 *                                                                           *
 * Returns the integer value of a string 'n'.                                *
 * ------------------------------------------------------------------------- */
int Menu::toInt(string n)
{
    return (int)strtol(n.c_str(), NULL, 10);
}

