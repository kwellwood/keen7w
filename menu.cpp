#include <iostream>
#include <fstream>
#include <allegro.h>
using namespace std;
#include "engine.h"
#include "menu.h"

/* ------------------------------------------------------------------------- *
 * Menu()                                                                    *
 *                                                                           *
 * Default constructor.                                                      *
 * ------------------------------------------------------------------------- */
Menu::Menu()
{   init(); }

/* ------------------------------------------------------------------------- *
 * void draw()                                                               *
 *                                                                           *
 * Draws the current menu state on the screen buffer.                        *
 * ------------------------------------------------------------------------- */
void Menu::draw()
{
    // --- draw current menu ---
    switch (_menu)
    {
        case MAIN: drawSprite(_menutile, 0, 0); break;
        case SAVE: drawSprite(_menutile+1, 0, 0); break;
        case LOAD: drawSprite(_menutile+2, 0, 0); break;
        case OPTIONS: drawSprite(_menutile+3, 0, 0); break;
    }

    // --- draw text for submenus ---
    if (_menu == LOAD || _menu == SAVE)
    {
        drawBox(126, 80, 210, 174, 146, EMPTY);
        drawBox(127, 81, 209, 173, 14, FILLED);
        // draw slot text here
        for (int slot=1; slot<=8; slot++)
        {   string name = savedGameTitle(slot);
            if (name == "") name = "[ Empty Slot ]";
            drawText(name.c_str(), 128, 70 + slot * 12, 131);
        }
    }
    else if (_menu == OPTIONS)
    {
        drawBox(126, 80, 228, 125, 146, EMPTY);
        drawBox(127, 81, 227, 124, 14, FILLED);
        // draw options stuff here
        string text = "OFF";
        if (getPlayerInvincible()) text = "ON";
        drawText(string("Invincibility: "+text).c_str(), 128, 82, 131);
        text = "OFF";
        drawText(string("Debug logging: "+text).c_str(), 128, 94, 131);
        text = "OFF";
        drawText(string("Show debug HUD: "+text).c_str(), 128, 106, 131);
        drawText("Enter cheat:", 128, 118, 131);
    }

    // --- display animated selector ---
    int optionspacing = _selectionpixels;
    int cursorx = _menux, cursory = _menuy;
    if (_menu == LOAD || _menu == SAVE || _menu == OPTIONS)
    {   optionspacing = 12; cursory = 80; }
    int frame = ((anmCounter()/_selectordelay) % _selectorframes) +
                _selectorstartfrm;
    drawSprite(frame, cursorx, cursory + ((_menuopt-1) * optionspacing));
}

/* ------------------------------------------------------------------------- *
 * void handleKey(int k)                                                     *
 *                                                                           *
 * Receives a keypress and performs the desired menu action.                 *
 * ------------------------------------------------------------------------- */
void Menu::handleKey(int k)
{
    // --- delay input ---
    if (timerTicks(HIRESTIMER) < _timermark) return;

    // --- come from the game out to menu ---
    if (gameMode() == INGAME)
    {
        if (getPlayerAction() == DYING) return;   // dont goto menu while dying
        //if dropdownstatus thingy not up then return
        setGameMode(INMENU);
        setGamePlay(PAUSED);
        while (key[k]);
        _timermark = timerTicks(HIRESTIMER) + (200 / 5);
        return;
    }

    switch (k)
    {
        // --- move selector up ---
        case KEY_UP:
        if (_menuopt > 1)
        {   _menuopt--; _timermark = timerTicks(HIRESTIMER) + (200 / 5); }
        break;
        // --- move selector down ---
        case KEY_DOWN:
        int maxitems;
        switch (_menu)
        {
            case MAIN: maxitems = MAINITEMS; break;
            case SAVE: maxitems = SAVELOADITEMS; break;
            case LOAD: maxitems = SAVELOADITEMS; break;
            case OPTIONS: maxitems = OPTIONSITEMS; break;
        }
        if (_menuopt < maxitems)
        {   _menuopt++; _timermark = timerTicks(HIRESTIMER) + (200 / 5); }
        break;
        // --- make a selection ---
        case KEY_ENTER:
        switch (_menu)
        {
            // -- main menu options --
            case MAIN:
            if      (_menuopt == 1) newGame();
            else if (_menuopt == 2)
            {   if (getPlayerAction() != DYING && gamePlay() != STOPPED)
                {   _menu = SAVE; _menuopt = 1; }
            }
            else if (_menuopt == 3) { _menu = LOAD; _menuopt = 1; }
            else if (_menuopt == 4) { _menu = OPTIONS; _menuopt = 1; }
            else if (_menuopt == 5) setQuitGame();
            break;
            // -- choose a save slot --
            case SAVE: saveGame(_menuopt); break;
            // -- choose a load slot --
            case LOAD: loadGame(_menuopt); break;
            // -- options menu options --
            case OPTIONS:
            if (_menuopt == 1) setPlayerInvincible(!getPlayerInvincible());
            else if (_menuopt == 2) { /*toggle logging*/ }
            else if (_menuopt == 3) { /*toggle debughud*/ }
            else if (_menuopt == 4) enterCheat();
            break;
        }
        while (key[k]);
        _timermark = timerTicks(HIRESTIMER) + (200 / 5);
        break;
        // --- move up in menu tree ---
        case KEY_ESC:
        if (_menu != MAIN)
        {   _menu = MAIN; _menuopt = 1; while (key[k]); }
        else if (getPlayerAction() != DYING && gamePlay() != STOPPED)
        {
            setGameMode(INGAME);
            if (pausedInGame() == 0) setGamePlay(RUNNING);
            while (key[k]);
        }
        _timermark = timerTicks(HIRESTIMER) + (200 / 5);
    }
}

/* ------------------------------------------------------------------------- *
 * void newGame()                                                            *
 *                                                                           *
 * Resets all variables for a new game and loads the first level.            *
 * ------------------------------------------------------------------------- */
void Menu::newGame()
{
    long mark = timerTicks(HIRESTIMER) + (1000 / 5);

    setPlayerLives(3);
    setPlayerDead(false);
    setPlayerAmmo(5);
    setPlayerScore(0);
    setPlayerNextlifeat(30000);
    setPlayerDrops(0);
    setPlayerAncientsrescued(0);
    clearLevelsComplete();

    setLevel(2);
    //drawLoadingBox(3);
    loadLevel(2);
    setGamePlay(RUNNING); setPausedingame(0);
    setMoveMode(MAPMODE);
    setGameMode(INGAME);

    while (timerTicks(HIRESTIMER) < mark);
}

/* ------------------------------------------------------------------------- *
 * void saveGame(int slot)                                                   *
 *   slot: save slot (from 1 to 8)                                           *
 *                                                                           *
 * Saves the current game into a file.                                       *
 * ------------------------------------------------------------------------- */
void Menu::saveGame(int slot)
{ }

/* ------------------------------------------------------------------------- *
 * void loadGame(int slot)                                                   *
 *   slot: load slot (from 1 to 8)                                           *
 *                                                                           *
 * Loads a saved game from a file.                                           *
 * ------------------------------------------------------------------------- */
void Menu::loadGame(int slot)
{ }

/* ------------------------------------------------------------------------- *
 * string savedGameTitle(int slot)                                           *
 *   slot: load slot to game name of                                         *
 *                                                                           *
 * Opens the saved game of the slot (if exists) and returns the name of it.  *
 * ------------------------------------------------------------------------- */
string Menu::savedGameTitle(int slot)
{   return ""; }

/* ------------------------------------------------------------------------- *
 * void drawLoadingBox(int level)                                            *
 *   level: level number to draw loading box for                             *
 *                                                                           *
 * Draws a loading box displaying the name of the level that is loading.     *
 * ------------------------------------------------------------------------- */
void Menu::drawLoadingBox(int level)
{
    drawSprite(_loadingtile, _loadx, _loady);
    
    // --- Get the text for entering a level ---
    char temp[256];
    string entermaptext;
    ifstream file("LEVELS.DAT");
    if (!file.is_open()) error("Can't open file LEVELS.DAT");
    file.getline(temp, 256);
    for (int i=1; i<= level; i++)
    {   file.getline(temp, 256);
        file.getline(temp, 256);
        file.getline(temp, 256); entermaptext = string(temp);
    }
    file.close();

    string line1, line2;
    breakUpText(entermaptext, line1, line2, 140);

    // print the line(s) in the loading box
    drawText(line1, 182 - getTextLen(line1) / 2, 78, 14);
    if (line2 != "")
    {   drawText(line2, 182 - getTextLen(line2) / 2, 88, 14); }

    blitToScreen();
}

/* ------------------------------------------------------------------------- *
 * void enterCheat()                                                         *
 *                                                                           *
 * Presents the player with a prompt to enter a cheat code.                  *
 * ------------------------------------------------------------------------- */
void Menu::enterCheat()
{ }

/* ------------------------------------------------------------------------- *
 * void tryAgainMenu()                                                       *
 *                                                                           *
 * Gives the user a prompt to try the level again or exit the level.         *
 * ------------------------------------------------------------------------- */
bool Menu::tryAgainMenu()
{
    drawSprite(_tryagaintile, 73, 57);

    drawText("You didn't make it past", 160 - 110 / 2, 67, 14);
    drawText(levelName(), 160-getTextLen(levelName())/2, 77, 14);
    drawText("Try Again", 160 - 46 / 2, 100, 14);
    drawText("Exit to Shadowlands", 160 - 92 / 2, 114, 14);
    blitToScreen();
    
    bool tryagain=true; int boxcol; long mark=0;
    do
    {
        if (timerTicked(FRAMETIMER))
        {
            boxcol = ((int)(timerTicks(HIRESTIMER) / 60) % 2) * 99 + 52;
            if (tryagain)
            {   drawBox(85, 96, 235, 109, boxcol, EMPTY);
                drawBox(86, 97, 234, 108, boxcol, EMPTY);
                drawBox(85,110, 235, 123, 240, EMPTY);
                drawBox(86,111, 234, 122, 240, EMPTY);
            }
            else
            {   drawBox(85, 96, 235, 109, 240, EMPTY);
                drawBox(86, 97, 234, 108, 240, EMPTY);
                drawBox(85,110, 235, 123, boxcol, EMPTY);
                drawBox(86,111, 234, 122, boxcol, EMPTY);
            }
            blitToScreen();
        }

        if (key[KEY_DOWN] && mark < timerTicks(HIRESTIMER))
        {   tryagain=false; mark = timerTicks(HIRESTIMER) + (100 / 5); }
        if (key[KEY_UP] && mark < timerTicks(HIRESTIMER))
        {   tryagain=true; mark = timerTicks(HIRESTIMER) + (100 / 5); }
    } while(!key[KEY_ENTER]);
    while(key[KEY_ENTER]);
    
    return tryagain;
}

/* ------------------------------------------------------------------------- *
 * void gotScubaGear()                                                       *
 *                                                                           *
 * Display Keen's dialog when he picks up the scuba gear.                    *
 * ------------------------------------------------------------------------- */
void Menu::gotScubaGear()
{
    drawSprite(_keenspeaktile, _keenspeakx, _keenspeaky);
    
    string speech = "Cool! I can breathe under water now!"; int line = 0;
    while (getTextLen(speech) > 144)
    {
        string textline;
        while (getTextLen(textline+speech.substr(0,speech.find(" ",0)+1))<=144
               && speech.find(" ",0) != string::npos)
        {   textline += speech.substr(0, speech.find(" ", 0) + 1);
            speech = speech.substr(speech.find(" ", 0)+1);
        }
        drawText(textline,140-getTextLen(textline)/2,78+line*10,14);
        line++;
    }
    drawText(speech,140-getTextLen(speech)/2,78+line*10,14);
    blitToScreen();
    
    // wait for keypress
    while(!key[KEY_ENTER] && !key[KEY_SPACE] &&
          !key[KEY_LCONTROL] && !key[KEY_RCONTROL]);
    // wait for the key to be lifted
    while(key[KEY_ENTER] || key[KEY_SPACE] ||
          key[KEY_LCONTROL] || key[KEY_RCONTROL]);
    
    drawSprite(_keenspeaktile+1, _keenspeakx, _keenspeaky);
    blitToScreen();
    
    // wait for keypress
    while(!key[KEY_ENTER] && !key[KEY_SPACE] &&
          !key[KEY_LCONTROL] && !key[KEY_RCONTROL]);
    // wait for the key to be lifted
    while(key[KEY_ENTER] || key[KEY_SPACE] ||
          key[KEY_LCONTROL] || key[KEY_RCONTROL]);
}

/* ------------------------------------------------------------------------- *
 * void cantSwim()                                                           *
 *                                                                           *
 * Display Keen's dialog when he tries to swim without scuba gear.           *
 * ------------------------------------------------------------------------- */
void Menu::cantSwim()
{
}

/* ------------------------------------------------------------------------- *
 * void helpMenu()                                                           *
 *                                                                           *
 * Display and run the help menu- instructions and story screens.            *
 * ------------------------------------------------------------------------- */
void Menu::helpMenu()
{
    int helpopt = 0;
    do
    {   do
        {   drawTile(_helpmenutile, 0, 0);
            drawSprite(_helpmenucursor, 54, 76 + (helpopt * 26));
            blitToScreen();
            
            if (timerTicks(HIRESTIMER) > _timermark)
            {
                if (key[KEY_UP])
                {   helpopt--;
                    if (helpopt < 0) helpopt = 0;
                    _timermark = timerTicks(HIRESTIMER) + (200 / 5);
                }
                if (key[KEY_DOWN])
                {   helpopt++;
                    if (helpopt > 1) helpopt = 1;
                    _timermark = timerTicks(HIRESTIMER) + (200 / 5);
                }
                if (key[KEY_ESC])
                {   while(key[KEY_ESC]); return; }
            }
        } while (!key[KEY_ENTER] || _timermark > timerTicks(HIRESTIMER));
        while(key[KEY_ENTER]);

        int basetile, page=0, maxpages;
        if (helpopt == 0) { basetile = _instructionstile; maxpages = 16 - 1; }
        if (helpopt == 1) { basetile = _storytile; maxpages = 9 - 1; }
        
        do
        {
            drawTile(basetile + page, 0, 0);
            blitToScreen();
            
            if (timerTicks(HIRESTIMER) > _timermark)
            {
                if (key[KEY_UP] || key[KEY_LEFT] || key[KEY_PGUP])
                {   page--;
                    if (page < 0) page = 0;
                    _timermark = timerTicks(HIRESTIMER) + (200 / 5);
                }
                if (key[KEY_DOWN] || key[KEY_RIGHT] || key[KEY_PGDN])
                {   page++;
                    if (page > maxpages) page = maxpages;
                    _timermark = timerTicks(HIRESTIMER) + (200 / 5);
                }
            }
        } while (!key[KEY_ESC] || _timermark > timerTicks(HIRESTIMER));
        while(key[KEY_ESC]);
    } while (true);
}

/* ------------------------------------------------------------------------- *
 * void ancientRescued()                                                     *
 *                                                                           *
 * Displays the dialog between keen and an ancient when he rescues one.      *
 * ------------------------------------------------------------------------- */
void Menu::ancientRescued()
{

}

/* ------------------------------------------------------------------------- *
 * void winGame()                                                            *
 *                                                                           *
 * Displays the endgame animation sequence with the oracle.                  *
 * ------------------------------------------------------------------------- */
void Menu::winGame()
{
    winGameScreen(3388,  22,   8,   2);
    winGameScreen(3389,   6,   8,   1);
    winGameScreen(3390,   6,   8,   1);
    winGameScreen(3391,   6,   8,   1);
    winGameScreen(3392,   6,   8,   1);
    winGameScreen(3393,   6,   8,   0);     // PAUSE
    winGameScreen(3394,  78,  30,   1);
    winGameScreen(3395,  28,  30,   0);     // PAUSE
    winGameScreen(3396,   6,   8,   4);
    winGameScreen(3397, 102,   8,   2);
    winGameScreen(3398,   8,   8,   0);     // PAUSE
    winGameScreen(3399,  22,   8, .75);
    winGameScreen(3400,  22,   8, .75);
    winGameScreen(3399,  22,   8, .75);
    winGameScreen(3400,  22,   8,   0);     // PAUSE
    winGameScreen(3401,   6,   8,   0);     // PAUSE
    winGameScreen(3402,   6,   8,   0);     // PAUSE
    winGameScreen(3403,   6,   8,   0);     // PAUSE
    winGameScreen(3404,   6,   8,   2);
    winGameScreen(3405, 126,   8,  .8);
    winGameScreen(3406, 126,   8,  .8);
    winGameScreen(3405, 126,   8,  .8);
    winGameScreen(3406, 126,   8,   0);     // PAUSE
    winGameScreen(3407,   6,   8,   1);
    winGameScreen(3408,   6,   8,   0);     // PAUSE
    winGameScreen(3409,   6,   8,   0);     // PAUSE
    winGameScreen(3410,   6,  48,   0);     // PAUSE
    winGameScreen(3411,   6, 101,   0);     // PAUSE
    winGameScreen(3412,  86,  68,   0);     // PAUSE
    winGameScreen(3413,  86,  88,   2);
    winGameScreen(3414,  86,  88,  .3);
    winGameScreen(3413,  86,  88,   0);     // PAUSE
    winGameScreen(3415,  86,  68,   0);     // PAUSE
    setGameOver();
}

/* ------------------------------------------------------------------------- *
 * void winGameScreen(int t, int x, int y, float delay)                      *
 *                                                                           *
 * Displays one frame of the ending sequence, pausing for a time or waiting  *
 * for a keypress.                                                           * 
 * ------------------------------------------------------------------------- */
void Menu::winGameScreen(int t, int x, int y, float delay)
{
    drawTile(3387, 0, 0);
    drawTile(t, x, y);
    blitToScreen();

    if (delay > 0)
    {   // wait specified time and then return
        long mark = timerTicks(HIRESTIMER);
        while (timerTicks(HIRESTIMER) < mark + (delay * 200));
        return;
    }
    else
    {   // wait for key, displaying blinking cursor updated at maxfps
        do
        {   while(!timerTicked(FRAMETIMER));
            drawSprite(3385+((int)(timerTicks(HIRESTIMER)/75)%2), 295, 185);
            blitToScreen();
        }while(!key[KEY_ENTER] && !key[KEY_SPACE] && !key[KEY_LCONTROL] &&
               !key[KEY_RCONTROL] && !key[KEY_ALT] && !key[KEY_ALTGR]);
        
        // wait for the key to be lifted
        while(key[KEY_ENTER] || key[KEY_SPACE] || key[KEY_LCONTROL] ||
              key[KEY_RCONTROL] || key[KEY_ALT] || key[KEY_ALTGR]);
    }
}

/* ------------------------------------------------------------------------- *
 * void init()                                                               *
 *                                                                           *
 * Loads menu data from a file and gets ready for use.                       *
 * ------------------------------------------------------------------------- */
void Menu::init()
{
    int tmp;
    ifstream file("MENU.DAT");
    if (!file.is_open()) error("Can't open file MENU.DAT");
    file >> tmp;
    if (tmp != MENU_VERSION)
    {   file.close(); error("Invalid version of MENU.DAT"); }
    file >> _menutile;
    file >> _selectorstartfrm;
    file >> _selectorframes;
    file >> _selectordelay;
    file >> _menux;
    file >> _menuy;
    file >> _selectionpixels;
    file >> _titlescreentile;
    file >> _helpmenutile;
    file >> _helpmenucursor;
    file >> _instructionstile;
    file >> _storytile;
    file >> _tryagaintile;
    file >> _loadingtile;
    file >> _loadx;
    file >> _loady;
    file >> _ancientspeaktile;
    file >> _ancspeakx;
    file >> _ancspeaky;
    file >> _keenspeaktile;
    file >> _keenspeakx;
    file >> _keenspeaky;
    file.close();
    
    _timermark = timerTicks(HIRESTIMER);
    _menu = MAIN;
    _menuopt = 1;
}