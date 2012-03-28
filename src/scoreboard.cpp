#include <cmath>
#include <allegro.h>
using namespace std;
#include "engine.h"
#include "player.h"
#include "scoreboard.h"

/* ------------------------------------------------------------------------- *
 * Scoreboard(Player* player)                                                *
 *   player: Player object to get stats from                                 *
 *                                                                           *
 * Default constructor.                                                      *
 * ------------------------------------------------------------------------- */
Scoreboard::Scoreboard(Player* player)
:_player(player)
{   init(); }

/* ------------------------------------------------------------------------- *
 * void tick()                                                               *
 *                                                                           *
 * Handles everything the scoreboard object does in one cycle of play.       *
 * ------------------------------------------------------------------------- */
void Scoreboard::tick()
{
    if (dropdownActive())
    {   if (_dropdownheight == 0)       // if status box reached the bottom
        {   while(key[KEY_ENTER]);
            // -----
            while(!key[KEY_ENTER] && !key[KEY_SPACE] && 
                  !key[KEY_LCONTROL] && !key[KEY_RCONTROL]);
            // -----
            while(key[KEY_ENTER] || key[KEY_SPACE] ||
                  key[KEY_LCONTROL] || key[KEY_RCONTROL]);

            _dropdownheight -= 15; _dropdownactivating = false;
        }
        else if (_dropdownactivating)   // keep the box moving down
        {   _dropdownheight += 15;
            setGamePlay(PAUSED);
            if (_dropdownheight > 0) _dropdownheight = 0;
        }
        else                            // box is on its way back up
        {   _dropdownheight -= 15;
            if (_dropdownheight < -181)
            {   _dropdownheight = -181; setGamePlay(RUNNING); }
        }
    }
}

/* ------------------------------------------------------------------------- *
 * void draw()                                                               *
 *                                                                           *
 * Draws the score box in the upper left of the screen.                      *
 * ------------------------------------------------------------------------- */
void Scoreboard::draw()
{
    drawTile(_scoreboardtile, _boardx, _boardy);
    
    int a = _player->ammo(); if (a > 99) a = 99;
    drawNumber(9, 6, 4, _player->score());
    drawNumber(2, 22, 20, _player->lives());
    drawNumber(2, 62, 20, a);
}

/* ------------------------------------------------------------------------- *
 * void drawDropdown()                                                       *
 *                                                                           *
 * Draws the dropdown status screen.                                         *
 * ------------------------------------------------------------------------- */
void Scoreboard::drawDropdown()
{
    drawSprite(_dropdowntile, 0, _dropdownheight);
    
    drawNumber(8, 72, 68 + _dropdownheight, _player->score());
    drawNumber(7, 176, 68 + _dropdownheight, _player->nextlifeat());
    drawNumber(3, 208, 108 + _dropdownheight, _player->ammo());
    drawNumber(2, 216, 124 + _dropdownheight, _player->drops());
    drawNumber(2, 120, 124 + _dropdownheight, _player->lives());

    if (_player->keyred()) drawSprite(1337, 119, 114 + _dropdownheight);
    if (_player->keyyel()) drawSprite(1338, 127, 114 + _dropdownheight);
    if (_player->keyblu()) drawSprite(1339, 135, 114 + _dropdownheight);
    if (_player->keygrn()) drawSprite(1340, 143, 114 + _dropdownheight);
    
    for (int a=0; a<_player->ancientsrescued(); a++)
    {   drawSprite(1341, 79 + (a * 8), 96 + _dropdownheight); }
    
    if (_player->scuba()) drawText("Scuba Gear", 86, 145+_dropdownheight, 14);

    // break the level name into two lines if its too long
    string line1, line2;
    breakUpText(levelName(), line1, line2, 160);

    // draw level name at top of status box
    if (line2 == "")
    {   drawText(line1,159-getTextLen(line1)/2,44+_dropdownheight,14);
    }
    else
    {   drawText(line1,159-getTextLen(line1)/2,40+_dropdownheight,14);
        drawText(line2,159-getTextLen(line2)/2,49+_dropdownheight,14);
    }

    blitToScreen();
}

/* ------------------------------------------------------------------------- *
 * void activateDropdown()                                                   *
 *                                                                           *
 * Starts the dropdown status box moving down.                               *
 * ------------------------------------------------------------------------- */
void Scoreboard::activateDropdown()
{   _dropdownheight += 15; _dropdownactivating = true; }

/* ------------------------------------------------------------------------- *
 * bool dropdownActive()                                                     *
 *                                                                           *
 * Returns true if the dropdown is active (on the screen.)                   *
 * ------------------------------------------------------------------------- */
bool Scoreboard::dropdownActive()
{   return _dropdownheight > -181; }

/* ------------------------------------------------------------------------- *
 * void drawNumber(int digits, int x, int y, int n)                          *
 *   digits: number of digits to display                                     *
 *   x,y   : location on screen                                              *
 *   n     : number to display                                               *
 *                                                                           *
 * Draws the score box in the upper left of the screen.                      *
 * ------------------------------------------------------------------------- */
void Scoreboard::drawNumber(int digits, int x, int y, int n)
{
    // step through the number from left to right and display each
    // digit with a corresponding pic of the digit in score font

    bool numberstarted=false;
    for (int d=digits-1; d>=0; d--)
    {   int a = (int)pow((float)10,(float)d); int digit = (int)(n / a);
        if (digit > 0 && d > 0)
        {   // - digit greater than zero found -
            drawTile(_scoredigit+digit,_boardx+x+((digits-1-d)*8),_boardy+y);
            n = n % (int)pow((float)10, (float)d);
            numberstarted=true;
        }
        else if (numberstarted)
        {   // - already found start of number -
            drawTile(_scoredigit+digit,_boardx+x+((digits-1-d)*8),_boardy+y);
            n = n % (int)pow((float)10, (float)d);
        }
        else if (d == 0)
        { drawTile(_scoredigit+digit,_boardx+x+((digits-1-d)*8),_boardy+y); }
    }
}

/* ------------------------------------------------------------------------- *
 * void init()                                                               *
 *                                                                           *
 * Loads scoreboard data from a file and gets ready for use.                 *
 * ------------------------------------------------------------------------- */
void Scoreboard::init()
{
    ifstream file("SCOREBOARD.DAT");
    if (!file.is_open()) error("Can't open file SCOREBOARD.DAT");
    
    file >> _scoreboardtile;
    file >> _scoredigit;
    file >> _boardx;
    file >> _boardy;
    file >> _dropdowntile;
    file.close();

    _dropdownheight = -181;
    _dropdownactivating = false;
}
