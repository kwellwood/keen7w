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

// map formats
#define CK7      0
#define K7W      1
#define MAP_FORMAT_IN  K7W
#define MAP_FORMAT_OUT K7W

// graphics mode
#define WINDOWED      0
#define FULLSCREEN    1

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

// tile layers
#define BACKGROUND 1
#define FOREGROUND 2

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

// Shot owner types
#define ENEMYSHOT   0
#define PLAYERSHOT  1

// drawinfo.mode modes (TED)
#define TILE_MODE   0
#define MATRIX_MODE 1
#define ITEM_MODE   2
#define ENEMY_MODE  3
#define DEL_MODE    4
#define PLAYER_MODE 5
#define SWTAG_MODE  6
#define PORTAL_MODE 7
#define LOCATR_MODE 8
#define LIFT_MODE   9

// drawinfo.portal modes (TED)
#define PLACE_ENTER 0
#define PLACE_EXIT  1

// --- Global drawing methods ---
void drawTile(int t, int x, int y);
void drawTile(int t, int x, int y, float size);
void drawSprite(int t, int x, int y);
void drawSprite(int t, int x, int y, float size);
void drawSprite(int t, int x, int y, int w, int h);
void drawSprite(int t, int x, int y, int maxdim);
void drawBox(int x1, int y1, int x2, int y2, int col, int type);
void drawLine(int x1, int y1, int x2, int y2, int col);         // TED ONLY
void drawText(string text, int x, int y, int col);
void drawBitmap(BITMAP* bitmap, int x, int y);
void grabBitmap(BITMAP* bitmap, int srcx, int srcy, int destx, int desty,
    int width, int height);
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
void setLevelComplete(int l);
void setGameComplete();
void setGameOver();
void setQuitGame();
bool getQuitGame();
void setLevel(int l);
int  getLevel();
void clearLevelsComplete();
int  getLevelComplete(int level);
string levelName(int level);
string levelName();
int  getTextLen(string str);

int    getNumTiles();                   // TED
int    tileType(int t);
int    tileType2(int t);
string tileDesc(int t);
string tileDesc2(int t);
int    slopeMask(int t);

int  mapWidth();
int  mapHeight();
string mapGetMusic();                   // TED
void   mapSetMusic(string music);       // TED
int  getMap(int x, int y);
int  getMapFG(int x, int y);
int  getSwitchTag(int x, int y);
int  getSlopeHeight(int x, int y, int l);
int  mapHitPoint(float x, float y);
bool getSwitch(int s);
void setMap(int x, int y, int t);
void setMapFG(int x, int y, int t);
void setSwitchTag(int x, int y, int sw);
void setSwitch(int s, bool v);
void mapToggleBridge(int loc);
void mapUnlockDoor(int x, int y);
void mapReplaceTile(int t1, int t2);    // TED
void mapNew(int w, int h);              // TED

void  focusCamera();
float getCameraOrigX();
float getCameraOrigY();
bool  onCamera(float x1, float y1, float x2, float y2);

void showGotScubaGear();
void showCantSwim();
void doAncientRescued();
int  getNumAncients();

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
int  getPlayerAncientsrescued();
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
void setPlayerScuba(bool s);
void togglePlayerJumpcheat();
bool playerCollision(int x1, int y1, int x2, int y2);
void playerReadMap(ifstream &file);
void playerWriteMap(ofstream &file);
void writePlayerSave(ofstream &file);
void readPlayerSave(ifstream &file);

// --- Miscellaneous global methods ---
void  loadLevel(int level);
void  startLevel(int level);
bool  timerTicked(int tmr);
long  timerTicks(int tmr);
void  breakUpText(string text, string &line1, string &line2, int maxlen);
void  waitForKey();
int   readqbInt(ifstream &file);
int   readqbLong(ifstream &file);
float readqbFloat(ifstream &file);
int   readInt(ifstream &file);
float readFloat(ifstream &file);
void  writeInt(ofstream &file, int value);
void  writeFloat(ofstream &file, float value);
void  error(string e);

// --- TED only methods ---
int  mouseX();
int  mouseY();
int  mouseXTile();
int  mouseYTile();
void trackMouse();
bool mouseLB();
bool mouseRB();
bool mouseIn(int x1, int y1, int x2, int y2);
bool mouseIn(int area[]);
void waitForLB();
void waitForRB();
void setDrawTile(int t);
int  getDrawTile();
void toggleDrawLayer(int l);
int  getDrawLayer();
int  getDir();
void setDir(int d);
int  getMode();
void setMode(int m);
void setItem(int i);
int  getItem();
void setEnemy(int e);
int  getEnemy();
void setTrack(int t);
int  getTrack();
void setMatrix(int m);
int  getMatrix();
int  getPortal();
void setPortal(int p);
void toggleGridsnap();
void toggleDarkenBG();
bool getGridsnap();
bool getDarkenBG();
bool loadMap(string mapfile);
void saveMap(string filename);
bool validFilename(string f);
int  numMatrices();
int  getMatrixAt(int m, int x, int y);
int  getMatrixOffsetX(int m);
int  getMatrixOffsetY(int m);
void setMatrixOffset(int m, int offx, int offy);
void setPlayerStart(int x, int y, int dir, bool putOnGround);
