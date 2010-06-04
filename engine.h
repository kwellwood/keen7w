#include <iostream>
#include <fstream>
#include <string>

#define SCALE    2
#define XRES     320*SCALE
#define YRES     200*SCALE
#define BPP      8
#define ts       16
#define maxfps   40
#define anmfps   11
#define ENGINE_VERSION 16

// graphics mode
#define WINDOWED      0
#define FULLSCREEN    1
//#define FORCEWINDOWED true

// -- tile type --
#define PASSIVE      0
#define WALL         1
#define SLOPE        2
#define HAZZARD      3
#define ASWITCH      4
#define POLE         5
#define ONEWAYFLOOR  6
#define GEMSOCKET    7          // foreground
#define FGSLOPE      8          // foreground slope (in Player::clipToMap)
#define FGWALL       9          // foreground wall
#define WATER       13
// -- tile type 2 --
#define OVERLAY      1          // foreground
#define ENDLEVEL    10
#define ENTERLEVEL  12
#define SCUBAGEAR   13
#define MORPHTILE   14

// movemode values
#define JUMPMODE   0
#define MAPMODE    1

// gamemode values
#define INMENU     1
#define INGAME     2

// gameplay values
#define RUNNING    1
#define PAUSED     2
#define STOPPED    3

// Hi speed Timers
#define FRAMETIMER 0
#define ANMTIMER   1
#define HIRESTIMER 2

// endLevel values
#define SUCCESSFUL   true
#define UNSUCCESSFUL false

// Movement directions
#define UP        0
#define DOWN      1
#define LEFT      2
#define RIGHT     3
#define UPLEFT    4
#define DOWNLEFT  5
#define UPRIGHT   6
#define DOWNRIGHT 7

// Player actions
#define MOVING      0
#define SHOOTING    1
#define SWITCHING   2
#define DYING       3
#define TELEPORTING 4
#define CLIMBING    5
#define POGO        6
#define HANGING     7
#define HANGCLIMB   8
#define IDLE        9
#define SWIMMING    10
#define LOOKUP      4
#define LOOKDOWN    5

// drawBox parameters
#define EMPTY       0
#define FILLED      1

// Bonus item IDs
#define SODA        1
#define GUM         2
#define CHOCOLATE   3
#define CANDY       4
#define DONUT       5
#define ICECREAM    6
#define FLASK       7
#define GUN         8
#define DROP        9
#define REDKEY     10
#define BLUKEY     11
#define GRNKEY     12
#define YELKEY     13

// Shot owner types
#define ENEMYSHOT   0
#define PLAYERSHOT  1

// --- Global drawing methods ---
void drawTile(int t, int x, int y);
void drawSprite(int t, int x, int y);
void drawBox(int x1, int y1, int x2, int y2, int col, int type);
void drawText(string text, int x, int y, int col);
void blitToScreen();

// --- Global accessor/modifier methods ---
int  anmCounter();
int  gameMode();
int  gamePlay();
int  moveMode();
int  pausedInGame();
void setGameMode(int gm);
void setGamePlay(int gp);
void setMoveMode(int mm);
void setPausedingame(int p);
void setLevelComplete();
void setGameComplete();
void setGameOver();
void setQuitGame();
void setLevel(int l);
void clearLevelsComplete();
string levelName(int level);
string levelName();
int  getTextLen(string str);

int  tileType(int t);
int  tileType2(int t);
int  slopeMask(int t);

int  mapWidth();
int  mapHeight();
int  getMap(int x, int y);
int  getMapFG(int x, int y);
int  getSwitchTag(int x, int y);
int  getSlopeHeight(int x, int y, int l);
int  mapHitPoint(float x, float y);
bool getSwitch(int s);
void setMap(int x, int y, int t);
void setMapFG(int x, int y, int y);
void setSwitch(int s, bool v);
void mapToggleBridge(int loc);
void mapUnlockDoor(int x, int y);

void  focusCamera();
float getCameraOrigX();
float getCameraOrigY();
bool  onCamera(float x1, float y1, float x2, float y2);

void showGotScubaGear();
void showCantSwim();
void doAncientRescued();

float getPlayerX();
float getPlayerY();
int  getPlayerXdir();
int  getPlayerAction();
bool getPlayerDead();
bool getPlayerInvincible();
int  getPlayerScore();
int  getPlayerNextlifeat();
int  getPlayerLives();
int  getPlayerAmmo();
int  getPlayerDrops();
int  getPlayerOnlift();
int  getPlayerOnBall();
void setPlayerLoc(float x, float y);
void setPlayerInvincible(bool i);
void setPlayerLives(int l);
void setPlayerDead(bool d);
void setPlayerAmmo(int a);
void setPlayerScore(int s);
void setPlayerNextlifeat(int n);
void setPlayerDrops(int d);
void setPlayerAncientsrescued(int a);
void setPlayerKeyred();
void setPlayerKeyblu();
void setPlayerKeygrn();
void setPlayerKeyyel();
bool playerCollision(int x1, int y1, int x2, int y2);
void playerReadMap(ifstream &file);

// --- Miscellaneous global methods ---
void  loadLevel(int level);
void  startLevel(int level);
bool  timerTicked(int tmr);
long  timerTicks(int tmr);
void  breakUpText(string text, string &line1, string &line2, int maxlen);
int   readInt(ifstream &file);
int   readLong(ifstream &file);
float readFloat(ifstream &file);
void  error(string e);