#include <string>
using namespace std;
#include "engine.h"
#include "audio.h"
#include "graphicslib.h"
#include "shot.h"
#include "player.h"
#include "map.h"
#include "camera.h"
#include "tedmenu.h"
#include "sprite.h"
#include "item.h"
#include "enemy.h"
#include "portal.h"
#include "lift.h"
#include "coredata.h"

class Matrix
{
    public:
    Matrix();
    ~Matrix();
    bool load(string file);
    int  get(int x, int y);
    int  offsetX();
    int  offsetY();
    int  width();
    int  height();
    void setOffset(int x, int y);
    
    private:
    int* _grid;
    int _width, _height;
    int _offsetx, _offsety;
};

// Declare all class data members
float Player::Friction;                                            // player.h
float Player::Gravity;                                             // player.h
vector<Sprite*> Sprite::Sprites;                                   // sprite.h
int Sprite::NextID;                                                // sprite.h
vector<Item*> Item::Items;                                         // item.h
float Enemy::Friction;                                             // enemy.h
int Enemy::NextID;                                                 // enemy.h
vector<Enemy*> Enemy::Enemies;                                     // enemy.h
enemyspriteframe Enemy::SpriteFrame[MAXENEMYFRAMES+1];             // enemy.h
vector<Portal> Portal::Portals;                                    // portal.h
int Portal::NextID;                                                // portal.h
int Lift::NextID = 0;                                              // lift.h
int Lift::Locator::NextID = 0;                                     // lift.h
int Lift::Liftstoptile, Lift::Liftlefttile, Lift::Liftrighttile;   // lift.h
int Lift::Liftuptile, Lift::Liftdowntile;                          // lift.h
vector<Lift> Lift::Lifts;                                          // lift.h
vector<Lift::Locator> Lift::Locators[16];                          // lift.h
int Shot::NextID;                                                  // shot.h
vector<Shot*> Shot::Shots;                                         // shot.h
bool Audio::Nosound = false;                                       // audio.h
int Audio::MusicVol = 255;                                         // audio.h
int Audio::SoundfxVol = 255;                                       // audio.h
ALMP3_MP3* Audio::Music = NULL;                                    // audio.h
DATAFILE* Audio::MusicData = NULL;                                 // audio.h
vector<SoundData> Audio::SoundFX;                                  // audio.h

bool   forcewindowed = false;
int    mapformatin = K7W;
int    mapformatout = K7W;
string tileset;                 // name of file containing tile list
string levelname;               // name of current level
bool   quitgame;

int  mousX, mousY;
bool mousLB, mousRB;

struct { int tile, layer, matrix, item, enemy, lifttrack, portal, mode, dir;
         bool gridsnap, darkenbg; } drawinfo;

// Various and sundry timers
struct Timer { long ticks; bool flag; } timer[3];
int anmcounter;

DATAFILE* fnt;                  // the font

int     graphicsmode;
BITMAP* scrbuf;                 // screen buffer
int     drawpage;               // current video page to use (fullscreen)
BITMAP* vidpage[2];             // video pages for flipping (fullscreen)
COLOR_MAP transtable;           // transparency table

GraphicsLib*    tilelib;
Map*            map;
Player*         player;
Menu*           menu;
TEDCamera*      camera;         // special camera for TED
vector<Matrix*> matrices;       // tile matrices collection

void closeButton();
void mainloop();
void doInput();
void drawScreen();
void paintTile(int l, int x, int y, int t);
void paintMatrix(int l, int x, int y, int m);
int  grabTile(int l, int x, int y);
void createItem(int x, int y, int i);
void createEnemy(int x, int y, int dir, int e, bool putOnGround);
void createLift(int x, int y, int track);
void createLocator(int x, int y, int track);
void createPortal(int x, int y, int part);
void delObject(int x, int y);
void doTimers();
void initGraphicsMode();
void initEngine();
void showMouse();
void hideMouse();
void tickFrameTimer();
void tickAnmTimer();
void tickHiresTimer();
void tickMusic();
void shutdown();
string toStr(int n);
