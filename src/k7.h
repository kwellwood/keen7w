#include <string>
using namespace std;
#include "engine.h"
#include "audio.h"
#include "graphicslib.h"
#include "shot.h"
#include "player.h"
#include "map.h"
#include "camera.h"
#include "menu.h"
#include "scoreboard.h"
#include "sprite.h"
#include "item.h"
#include "enemy.h"
#include "portal.h"
#include "lift.h"
#include "coredata.h"

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
int Portal::NextID = 0;                                            // portal.h
vector<Portal> Portal::Portals;                                    // portal.h
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
