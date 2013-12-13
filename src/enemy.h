#include <iostream>
#include <fstream>
#include <vector>
using namespace std;
#include "engine.h"

#define MAXENEMYFRAMES 68

// Frames of the stars stunned anm
#define STARS_START   553
#define STARS_END     555

// Enemy types for collisions
#define NOTANENEMY   -1
#define NPCENEMY      0
#define DEADLYENEMY   1
#define ENDLEVELENEMY 2

// Enemy Types
#define SLUG          1
#define BOUNDER       2
#define MUSHROOM      3
#define LICK          4
#define ARACHNUT      5
#define WORM          6
#define SNAKE         7
#define ANCIENT       8
#define SPEARUP       9
#define SPEARDOWN    10
#define SPEARLEFT    11
#define SPEARRIGHT   12
#define SHOOTERUP    13
#define SHOOTERDOWN  14
#define SHOOTERLEFT  15
#define SHOOTERRIGHT 16
#define MIMROCK      17
#define CLOUD        18

// Enemy status and actions
#define ENEMYDEAD      10
#define MUSHBOUNCELO   1            // Mushroom         status
#define MUSHBOUNCEHI   2            //                  status
#define SLUGCRAWLING   0            // Slug             status
#define SLUGCRAWL1     0            //                  action
#define SLUGCRAWL2     1            //                  action
#define SLUGPOOPING    2            //                  status/action
#define BALLBOUNCEUP   0            // Bounder          status
#define BALLFOLLOW     1            //                  status
#define LICKSIT        0            // Lick             status
#define LICKATTACK     1            //                  status
#define LICKFOLLOW     2            //                  status
#define LICKFIRESML    1            //                  action
#define LICKFIREMED    2            //                  action
#define LICKFIREBIG    3            //                  action
#define ARACHCRAWLING  0            // Arachnut         status
#define ARACHSTUNNED   1            //                  status
#define ARACHWAKING    2            //                  status
#define WORMCRAWL1     0            // Worm             status
#define WORMCRAWL2     1            //                  status
#define SNAKEMOVE      0            // Snake            status
#define SNAKEPEEK      1            //
#define SNAKEATTACK    2            //
#define OLDMANWALKING  0            // Ancient          status
#define OLDMANBROODING 1            //                  status/action
#define OLDMANWALK1    0            //                  action
#define OLDMANWALK2    1            //                  action
#define OLDMANBROOD    2            //
#define SPEARWAIT      0            // Spear            status
#define SPEARATTACK    1            //
#define SHOOTERWAIT    0            // Arrow Shooter    status
#define MIMROCKSIT     0            // Mim Rock         status
#define MIMROCKSNEAK   1            //
#define MIMROCKJUMP    2            //
#define MIMROCKONHEAD  3            //
#define CLOUDSIT       0            // Cloud            status
#define CLOUDROAM      1            //
#define CLOUDSTRIKE    2            //

struct enemyspriteframe { int tile; int width; int height; };

class Enemy
{
    public:
    static void init();
    static void addEnemy(Enemy* e);
    static void delEnemy(int id);
    static Enemy* getEnemyByID(int id);
    static void tickEnemies();
    static void drawEnemies();
    static int  collideEnemy(int x1, int y1, int x2, int y2);
    static int  collideEnemyID(int x1, int y1, int x2, int y2);
    static int  standingOnEnemy(float x, float y);
    static void clearEnemies();
    static void readEnemies(ifstream &file);
    static void writeMap(ofstream &file);
    static void writeSave(ofstream &file);
    static void readSave(ifstream &file);
    static void createEnemy(int x, int y, int dir, int type, bool putOnGround);
    static string getName(int type);
    static int    getIcon(int type, int dir);
    
    static float Friction;
    static int   NextID;
    
    float top();
    void  draw(float zoom, int vpx, int vpy);
    
    protected:
    static vector<Enemy*> Enemies;
    static enemyspriteframe SpriteFrame[MAXENEMYFRAMES+1];

    Enemy(int type, float x, float y, int xdir, int action, int actiontime,
        int status, bool onground, int frameslot, int xsize, int ysize,
        float xmax, float ymax, float gravity);
    Enemy() {}
    virtual int  collision(int x1, int y1, int x2, int y2);
    bool collisionb(int x1, int y1, int x2, int y2);
    virtual void tick();
    virtual void collideWithShots();
    virtual void kill();
    virtual bool stoodUpon(float x, float y);
    virtual void doPhysics();
    virtual void doAI() = 0;
    virtual void updateFrame() = 0;
    void doOnGround();
    void hitWall(int p, float hx, float hy);
    void hitSlope(float x, float y, int l);
    bool holeHere(int mx, int my);
    virtual void dropToGround();
    bool inRange(int dist);
    bool inRangeY(int dist);
    bool betweenRangesX(int x1, int x2);
    void updateClipPoints();

    int   _type, _id;
    float _x, _y, _xv, _yv, _xmax, _ymax, _gravity;
    bool  _onground, _onslope;
    int   _xdir, _action, _actiontime, _status, _statustime, _xsize, _ysize;
    int   _frame, _txsize, _tysize, _starsprite;
    
    struct coordinate { float x; float y; } _clippoint[5];
};

class TEDEnemy : public Enemy
{
    public:
    TEDEnemy():Enemy() { };
    static void drawEnemies(float zoom, int vpx, int vpy);
};

class SpearEnemy : public Enemy
{
    public:
    SpearEnemy(int type, float x, float y, int frameslot, int xsize,
        int ysize);
    SpearEnemy() {}
    int  collision(int x1, int y1, int x2, int y2);
    void tick();
};

class ShooterEnemy : public Enemy
{
    public:
    ShooterEnemy(int type, float x, float y, int frameslot, int xsize,
        int ysize);
    ShooterEnemy() {}
    void tick();
};

class Slug : public Enemy
{
    public:
    static int icon(int dir)
    {   if (dir == LEFT) return SpriteFrame[0].tile;
        else return SpriteFrame[3].tile; }
    Slug(float x, float y, int dir);
    Slug() {}
    void doAI();
    void updateFrame();
};

class Bounder : public Enemy
{
    public:
    static int icon(int dir)
    {   if (dir == LEFT) return SpriteFrame[10].tile;
        else return SpriteFrame[12].tile; }
    Bounder(float x, float y);
    Bounder() {}
    int  collision(int x1, int y1, int x2, int y2);
    bool stoodUpon(float x, float y);
    void doAI();
    void updateFrame();
};

class Mushroom : public Enemy
{
    public:
    static int icon(int dir)
    {   if (dir == LEFT) return SpriteFrame[17].tile;
        else return SpriteFrame[19].tile; }
    Mushroom(float x, float y);
    Mushroom() {}
    void collideWithShots();
    void doAI();
    void updateFrame();
};

class Lick : public Enemy
{
    public:
    static int icon(int dir)
    {   if (dir == LEFT) return SpriteFrame[21].tile;
        else return SpriteFrame[22].tile; }
    Lick(float x, float y, int dir);
    Lick() {}
    int  collision(int x1, int y1, int x2, int y2);
    void doAI();
    void updateFrame();
};

class Arachnut : public Enemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[30].tile; }
    Arachnut(float x, float y, int dir);
    Arachnut() {}
    int  collision(int x1, int y1, int x2, int y2);
    void kill();
    void doAI();
    void updateFrame();
};

class Worm : public Enemy
{
    public:
    static int icon(int dir)
    {   if (dir == LEFT) return SpriteFrame[32].tile;
        else return SpriteFrame[33].tile; }
    Worm(float x, float y, int dir);
    Worm() {}
    int  collision(int x1, int y1, int x2, int y2);
    void collideWithShots();
    void doAI();
    void updateFrame();
};

class Snake : public Enemy
{
    public:
    static int icon(int dir)
    {   if (dir == LEFT) return SpriteFrame[37].tile;
        else return SpriteFrame[39].tile; }
    Snake(float x, float y, int dir);
    Snake() {}
    int  collision(int x1, int y1, int x2, int y2);
    void collideWithShots();
    void doAI();
    void updateFrame();
};

class Ancient : public Enemy
{
    public:
    static int icon(int dir)
    {   if (dir == LEFT) return SpriteFrame[42].tile;
        else return SpriteFrame[44].tile; }
    Ancient(float x, float y, int dir);
    Ancient() {}
    int  collision(int x1, int y1, int x2, int y2);
    void collideWithShots();
    void doAI();
    void updateFrame();
};

class SpearUp : public SpearEnemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[47].tile; }
    SpearUp(float x, float y);
    SpearUp() {}
    void doAI();
    void updateFrame();
};

class SpearDown : public SpearEnemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[49].tile; }
    SpearDown(float x, float y);
    SpearDown() {}
    void doAI();
    void updateFrame();
    void dropToGround();
};

class SpearLeft : public SpearEnemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[51].tile; }
    SpearLeft(float x, float y);
    SpearLeft() {}
    void doAI();
    void updateFrame();
    void dropToGround();
};

class SpearRight : public SpearEnemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[53].tile; }
    SpearRight(float x, float y);
    SpearRight() {}
    void doAI();
    void updateFrame();
    void dropToGround();
};

class ShooterUp : public ShooterEnemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[54].tile; }
    ShooterUp(float x, float y);
    ShooterUp() {}
    void doAI();
    void updateFrame();
};

class ShooterDown : public ShooterEnemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[55].tile; }
    ShooterDown(float x, float y);
    ShooterDown() {}
    void doAI();
    void updateFrame();
    void dropToGround();
};

class ShooterLeft : public ShooterEnemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[56].tile; }
    ShooterLeft(float x, float y);
    ShooterLeft() {}
    void doAI();
    void updateFrame();
    void dropToGround();
};

class ShooterRight : public ShooterEnemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[57].tile; }
    ShooterRight(float x, float y);
    ShooterRight() {}
    void doAI();
    void updateFrame();
    void dropToGround();
};

class MimRock : public Enemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[58].tile; }
    MimRock(float x, float y, int dir);
    MimRock() {}
    int  collision(int x1, int y1, int x2, int y2);
    void collideWithShots();
    void doAI();
    void updateFrame();
};

class Cloud : public Enemy
{
    public:
    static int icon(int dir)
    {   return SpriteFrame[67].tile; }
    Cloud(float x, float y, int dir);
    Cloud() {}
    int collision(int x1, int y1, int x2, int y2);
    void collideWithShots();
    void doAI();
    void updateFrame();
    void dropToGround();
};
