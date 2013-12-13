#include <string>
using namespace std;
#include "graphicslib.h"
#include "player.h"
#include "map.h"
#include "camera.h"
#include "menu.h"
#include "scoreboard.h"

bool   forcewindowed = false;
string tileset;                 // name of file containing tile list
int    totalAncients;           // ancients needed to be rescued to win
int    movemode;                // overhead map(mapmode) or platform(jumpmode)
int    gamemode;                // inmenu or ingame
int    gameplay;                // running, stopped or paused
int    pausedingame;
int    curlevel;                // current level being played
string levelname;               // name of current level
bool   gamecompleteflag;
bool*  levelsComplete;          // array for completed levels
bool   gameoverflag;
bool   quitgame;

// Various and sundry timers
struct Timer { long ticks; bool flag; } timer[3];
int anmcounter;

DATAFILE*    fnt;               // the font

int graphicsmode;
BITMAP* scrbuf;                 // screen buffer
int     drawpage;               // current video page to use (fullscreen)
BITMAP* vidpage[2];             // video pages for flipping (fullscreen)

GraphicsLib* tilelib;
Map*         map;
Player*      player;
Menu*        menu;
Camera*      camera;
Scoreboard*  scoreboard;

void closeButton();
void mainloop();
void doInput();
void drawScreen();
void tryAgainMenu();
void endLevel(bool successful);
void prepareOverheadMap();
void endGame();
void winGame();
void doTimers();
void initGraphicsMode();
void initEngine();
void tickFrameTimer();
void tickAnmTimer();
void tickHiresTimer();
void tickMusic();
void doTitleScreen();
void shutdown();
