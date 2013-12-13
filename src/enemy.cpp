#include <allegro.h>
using namespace std;
#include "engine.h"
#include "audio.h"
#include "shot.h"
#include "sprite.h"
#include "enemy.h"

float Enemy::Friction = 0.0;
int Enemy::NextID = 0;
vector<Enemy*> Enemy::Enemies;
enemyspriteframe Enemy::SpriteFrame[MAXENEMYFRAMES+1];

/* ------------------------------------------------------------------------- *
 * static void init()                                                        *
 *                                                                           *
 * Initialize the enemy system by reading enemy frames from enemies.cfg      *
 * ------------------------------------------------------------------------- */
void Enemy::init()
{
    clearEnemies();
    ifstream file("enemies.cfg");
    if (!file.is_open())
    {   error("Can't open file enemies.cfg"); }
    for (int f=0; f<=MAXENEMYFRAMES; f++)
    {
        file >> SpriteFrame[f].tile;            // frame's tile number
        file >> SpriteFrame[f].width;           // frame's width
        file >> SpriteFrame[f].height;          // frame's height
    }
    file.close();
}

/* ------------------------------------------------------------------------- *
 * static void addEnemy(Enemy* e)                                            *
 *   e: pointer to new enemy to add                                          *
 *                                                                           *
 * Adds a new enemy to the enemies collection.                               *
 * ------------------------------------------------------------------------- */
void Enemy::addEnemy(Enemy* e)
{   Enemies.push_back(e); }

/* ------------------------------------------------------------------------- *
 * static void delEnemy(int id)                                              *
 *   id: enemy's unique id number                                            *
 *                                                                           *
 * Removes an enemy from the collection. Used by TED.                        *
 * ------------------------------------------------------------------------- */
void Enemy::delEnemy(int id)
{
    int slot = -1;                  // slot in Enemies where enemy ojbect is
    for (int e=0; e<Enemies.size(); e++)
    {   if (Enemies[e]->_id == id) slot = e; }
    
    if (slot < 0) return;           // enemy with id wasnt found, else slot
                                    // contains its location in Enemies
    // Delete the object and remove its pointer
    // from the Enemies vector
    delete Enemies[slot];
    vector<Enemy*>::iterator iter = Enemies.begin();
    while (slot > 0) { iter++; slot--; }
    Enemies.erase(iter);
}

/* ------------------------------------------------------------------------- *
 * static Enemy* getEnemyByID(int id)                                        *
 *   id: an enemy's unique id                                                *
 *                                                                           *
 * Finds an enemy in the collection by its id and returns it.                *
 * ------------------------------------------------------------------------- */
Enemy* Enemy::getEnemyByID(int id)
{
    for (vector<Enemy*>::iterator i=Enemies.begin(); i!=Enemies.end(); i++)
    {   if ((*i)->_id == id) return (*i); }
    return NULL;
}

/* ------------------------------------------------------------------------- *
 * static void tickEnemies()                                                 *
 *                                                                           *
 * Handles enemy physics, AI and animation frames for one game cycle.        *
 * ------------------------------------------------------------------------- */
void Enemy::tickEnemies()
{   for (int e=0; e<Enemies.size(); e++) Enemies[e]->tick(); }

/* ------------------------------------------------------------------------- *
 * static void drawEnemies()                                                 *
 *                                                                           *
 * Draws all enemies in the enemy collection.                                *
 * ------------------------------------------------------------------------- */
void Enemy::drawEnemies()
{   for (int e=0; e<Enemies.size(); e++) Enemies[e]->draw(1, 0, 0); }

/* ------------------------------------------------------------------------- *
 * static int collideEnemy(int x1, int y1, int x2, int y2)                   *
 *   x1,y1,x2,y2: collision rectangle to check                               *
 *                                                                           *
 * Checks all the enemies for a collision with the given area and returns    *
 * the type of the most important collision found. see below for details.    *
 * Returns NOTANENEMY if no collions with enemies.                           *
 * ------------------------------------------------------------------------- */
int Enemy::collideEnemy(int x1, int y1, int x2, int y2)
{
    // in order of priority should player collide with multiple enemies:
    // 1. DEADLYENEMY
    // 2. ENDLEVELENEMY
    // 3. NPCENEMY
    int collisiontype = NOTANENEMY;
    for (int e=0; e<Enemies.size(); e++)
    {   if (Enemies[e]->_status != ENEMYDEAD)
        {   int temp = Enemies[e]->collision(x1, y1, x2, y2);
            if (temp == DEADLYENEMY) return DEADLYENEMY;
            if (temp == ENDLEVELENEMY) collisiontype = ENDLEVELENEMY;
            if (temp == NPCENEMY && collisiontype != ENDLEVELENEMY)
                collisiontype = NPCENEMY;
        }
    }
    return collisiontype;
}

/* ------------------------------------------------------------------------- *
 * static int collideEnemyID(int x1, int y1, int x2, int y2)                 *
 *   x1,y1,x2,y2: collision rectangle to check                               *
 *                                                                           *
 * Checks all the enemies for a collision with the given area and returns    *
 * the ID of the first enemy collision found. Returns -1 if no collition     *
 * found. Used by TED.                                                       *
 * ------------------------------------------------------------------------- */
int Enemy::collideEnemyID(int x1, int y1, int x2, int y2)
{
    for (int e=0; e<Enemies.size(); e++)
    {   if (Enemies[e]->collisionb(x1, y1, x2, y2)) return Enemies[e]->_id; }
    return -1;
}

/* ------------------------------------------------------------------------- *
 * static int standingOnEnemy(float x, float y)                              *
 *   x,y: location of feet to check                                          *
 *                                                                           *
 * Returns the id of the first enemy found to be being stood on, or -1 if    *
 * no enemy is being stood upon.                                             *
 * ------------------------------------------------------------------------- */
int Enemy::standingOnEnemy(float x, float y)
{   for (int e=0; e<Enemies.size(); e++)
    {   if (Enemies[e]->stoodUpon(x, y)) return Enemies[e]->_id; }
    return -1;
}

/* ------------------------------------------------------------------------- *
 * static void clearEnemies()                                                *
 *                                                                           *
 * Clears all enemies from the enemy collection.                             *
 * ------------------------------------------------------------------------- */
void Enemy::clearEnemies()
{   for (int e=0; e<Enemies.size(); e++) delete Enemies[e];
    Enemies.clear(); NextID = 0;
}

/* ------------------------------------------------------------------------- *
 * static void readEnemies(ifstream &file)                                   *
 *   file: file stream to read                                               *
 *                                                                           *
 * Read the enemies from an open map file.                                   *
 * ------------------------------------------------------------------------- */
void Enemy::readEnemies(ifstream &file)
{
    clearEnemies();

    int numenemies;
    if (MAP_FORMAT_IN) { file.read((char*)&numenemies, 4); }
    else { numenemies = readqbInt(file); }
    
    for (int e=0; e<numenemies; e++)
    {   float x, y; int type, dir;
        if (MAP_FORMAT_IN == K7W)
        {   file.read((char*)&x, 4); file.read((char*)&y, 4);
            file.read((char*)&dir, 4);
            file.read((char*)&type, 4); }
        else
        {   x = readqbFloat(file); y = readqbFloat(file);
            type = readqbInt(file); dir = LEFT; }

        switch (type)
        {   case SLUG: addEnemy(new Slug(x, y, dir)); break;
            case BOUNDER: addEnemy(new Bounder(x, y)); break;
            case MUSHROOM: addEnemy(new Mushroom(x, y)); break;
            case LICK: addEnemy(new Lick(x, y, dir)); break;
            case ARACHNUT: addEnemy(new Arachnut(x, y, dir)); break;
            case WORM: addEnemy(new Worm(x, y, dir)); break;
            case SNAKE: addEnemy(new Snake(x, y, dir)); break;
            case ANCIENT: addEnemy(new Ancient(x, y, dir)); break;
            case SPEARUP: addEnemy(new SpearUp(x, y)); break;
            case SPEARDOWN: addEnemy(new SpearDown(x, y)); break;
            case SPEARLEFT: addEnemy(new SpearLeft(x, y)); break;
            case SPEARRIGHT: addEnemy(new SpearRight(x, y)); break;
            case SHOOTERUP: addEnemy(new ShooterUp(x, y)); break;
            case SHOOTERDOWN: addEnemy(new ShooterDown(x, y)); break;
            case SHOOTERLEFT: addEnemy(new ShooterLeft(x, y)); break;
            case SHOOTERRIGHT: addEnemy(new ShooterRight(x, y)); break;
            case MIMROCK: addEnemy(new MimRock(x, y, dir)); break;
            case CLOUD: addEnemy(new Cloud(x, y, dir)); break;
            default: error("Unrecognized enemy type");
        }
        Enemy* enemy = getEnemyByID(NextID-1); enemy->updateFrame();
    }
}

/* ------------------------------------------------------------------------- *
 * static void writeMap(ofstream &file)                                      *
 *   file: file stream to write to                                           *
 *                                                                           *
 * Writes the enemies to an open map file.                                   *
 * ------------------------------------------------------------------------- */
void Enemy::writeMap(ofstream &file)
{
    int temp = Enemies.size(); file.write((char*)&temp, 4);
    
    for (int i=0; i<Enemies.size(); i++)
    {   file.write((char*)&Enemies[i]->_x, 4);
        file.write((char*)&Enemies[i]->_y, 4);
        file.write((char*)&Enemies[i]->_xdir, 4);
        file.write((char*)&Enemies[i]->_type, 4);
    }
}

/* ------------------------------------------------------------------------- *
 * static void writeSave(ofstream &file)                                     *
 *   file: file stream to write to                                           *
 *                                                                           *
 * Writes out all the enemies for a saved game file.                         *
 * ------------------------------------------------------------------------- */
void Enemy::writeSave(ofstream &file)
{
    writeInt(file, Enemies.size());

    for (int e=0; e<Enemies.size(); e++)
    {   int type = Enemies[e]->_type;
        writeInt(file, type);
        switch(type)
        {   case SLUG:
            file.write((char*)Enemies[e], sizeof(Slug)); break;
            case BOUNDER:
            file.write((char*)Enemies[e], sizeof(Bounder)); break;
            case MUSHROOM:
            file.write((char*)Enemies[e], sizeof(Mushroom)); break;
            case LICK:
            file.write((char*)Enemies[e], sizeof(Lick)); break;
            case ARACHNUT:
            file.write((char*)Enemies[e], sizeof(Arachnut)); break;
            case WORM:
            file.write((char*)Enemies[e], sizeof(Worm)); break;
            case SNAKE:
            file.write((char*)Enemies[e], sizeof(Snake)); break;
            case ANCIENT:
            file.write((char*)Enemies[e], sizeof(Ancient)); break;
            case SPEARUP:
            file.write((char*)Enemies[e], sizeof(SpearUp)); break;
            case SPEARDOWN:
            file.write((char*)Enemies[e], sizeof(SpearDown)); break;
            case SPEARLEFT:
            file.write((char*)Enemies[e], sizeof(SpearLeft)); break;
            case SPEARRIGHT:
            file.write((char*)Enemies[e], sizeof(SpearRight)); break;
            case SHOOTERUP:
            file.write((char*)Enemies[e], sizeof(ShooterUp)); break;
            case SHOOTERDOWN:
            file.write((char*)Enemies[e], sizeof(ShooterDown)); break;
            case SHOOTERLEFT:
            file.write((char*)Enemies[e], sizeof(ShooterLeft)); break;
            case SHOOTERRIGHT:
            file.write((char*)Enemies[e], sizeof(ShooterRight)); break;
            case MIMROCK:
            file.write((char*)Enemies[e], sizeof(MimRock)); break;
            case CLOUD:
            file.write((char*)Enemies[e], sizeof(Cloud)); break;
        }
    }
}

/* ------------------------------------------------------------------------- *
 * static void readSave(ifstream &file)                                      *
 *   file: file stream to read from                                          *
 *                                                                           *
 * Reads in all the enemies from a saved game file.                          *
 * ------------------------------------------------------------------------- */
void Enemy::readSave(ifstream &file)
{
    clearEnemies();
    int numenemies = readInt(file);
    NextID = 0;
    
    for (int i=0; i<numenemies; i++)
    {   int type = readInt(file);
        Enemy* e;
        switch (type)
        {   case SLUG: e = new Slug();
            file.read((char*)e, sizeof(Slug)); addEnemy(e); break;
            case BOUNDER: e = new Bounder();
            file.read((char*)e, sizeof(Bounder)); addEnemy(e); break;
            case MUSHROOM: e = new Mushroom();
            file.read((char*)e, sizeof(Mushroom)); addEnemy(e); break;
            case LICK: e = new Lick();
            file.read((char*)e, sizeof(Lick)); addEnemy(e); break;
            case ARACHNUT: e = new Arachnut();
            file.read((char*)e, sizeof(Arachnut)); addEnemy(e); break;
            case WORM: e = new Worm();
            file.read((char*)e, sizeof(Worm)); addEnemy(e); break;
            case SNAKE: e = new Snake();
            file.read((char*)e, sizeof(Snake)); addEnemy(e); break;
            case ANCIENT: e = new Ancient();
            file.read((char*)e, sizeof(Ancient)); addEnemy(e); break;
            case SPEARUP: e = new SpearUp();
            file.read((char*)e, sizeof(SpearUp)); addEnemy(e); break;
            case SPEARDOWN: e = new SpearDown();
            file.read((char*)e, sizeof(SpearDown)); addEnemy(e); break;
            case SPEARLEFT: e = new SpearLeft();
            file.read((char*)e, sizeof(SpearLeft)); addEnemy(e); break;
            case SPEARRIGHT: e = new SpearRight();
            file.read((char*)e, sizeof(SpearRight)); addEnemy(e); break;
            case SHOOTERUP: e = new ShooterUp();
            file.read((char*)e, sizeof(ShooterUp)); addEnemy(e); break;
            case SHOOTERDOWN: e = new ShooterDown();
            file.read((char*)e, sizeof(ShooterDown)); addEnemy(e); break;
            case SHOOTERLEFT: e = new ShooterLeft();
            file.read((char*)e, sizeof(ShooterLeft)); addEnemy(e); break;
            case SHOOTERRIGHT: e = new ShooterRight();
            file.read((char*)e, sizeof(ShooterRight)); addEnemy(e); break;
            case MIMROCK: e = new MimRock();
            file.read((char*)e, sizeof(MimRock)); addEnemy(e); break;
            case CLOUD: e = new Cloud();
            file.read((char*)e, sizeof(Cloud)); addEnemy(e); break;
        }
        if (e->_id >= NextID) NextID = e->_id + 1;
    }
}

/* ------------------------------------------------------------------------- *
 * static void createEnemy(int x, int y, int dir, int type, bool putOnGround)*
 *   x,y : location of the item                                              *
 *   type: species of the enemy                                              *
 *   dir : direction of the enemy                                            *
 *   putOnGround: whether to drop the enemy to the ground or not             *
 *                                                                           *
 * Spawns a new enemy of type 'type' at location 'x,y'                       *
 * ------------------------------------------------------------------------- */
void Enemy::createEnemy(int x, int y, int dir, int type, bool putOnGround)
{
    switch (type)
    {   case SLUG: addEnemy(new Slug(x, y, dir)); break;
        case BOUNDER: addEnemy(new Bounder(x, y)); break;
        case MUSHROOM: addEnemy(new Mushroom(x, y)); break;
        case LICK: addEnemy(new Lick(x, y, dir)); break;
        case ARACHNUT: addEnemy(new Arachnut(x, y, dir)); break;
        case WORM: addEnemy(new Worm(x, y, dir)); break;
        case SNAKE: addEnemy(new Snake(x, y, dir)); break;
        case ANCIENT: addEnemy(new Ancient(x, y, dir)); break;
        case SPEARUP: addEnemy(new SpearUp(x, y)); break;
        case SPEARDOWN: addEnemy(new SpearDown(x, y)); break;
        case SPEARLEFT: addEnemy(new SpearLeft(x, y)); break;
        case SPEARRIGHT: addEnemy(new SpearRight(x, y)); break;
        case SHOOTERUP: addEnemy(new ShooterUp(x, y)); break;
        case SHOOTERDOWN: addEnemy(new ShooterDown(x, y)); break;
        case SHOOTERLEFT: addEnemy(new ShooterLeft(x, y)); break;
        case SHOOTERRIGHT: addEnemy(new ShooterRight(x, y)); break;
        case MIMROCK: addEnemy(new MimRock(x, y, dir)); break;
        case CLOUD: addEnemy(new Cloud(x, y, dir)); break;
        default: return;
    }
    
    Enemy* enemy = getEnemyByID(NextID-1);
    enemy->updateFrame();
    if (putOnGround)
    {   enemy->dropToGround(); }
}

/* ------------------------------------------------------------------------- *
 * static string getName(int type)                                           *
 *   type: type of enemy                                                     *
 *                                                                           *
 * Returns a string description of an enemy by type.                         *
 * ------------------------------------------------------------------------- */
string Enemy::getName(int type)
{
    switch (type)
    {   case SLUG: return "Slug";
        case BOUNDER: return "Bounder";
        case MUSHROOM: return "Mushroom";
        case LICK: return "Lick";
        case ARACHNUT: return "Arachnut";
        case WORM: return "Worm";
        case SNAKE: return "Snake";
        case ANCIENT: return "Ancient";
        case SPEARUP: return "Spear (Up)";
        case SPEARDOWN: return "Spear (Down)";
        case SPEARLEFT: return "Spear (Left)";
        case SPEARRIGHT: return "Spear (Right)";
        case SHOOTERUP: return "Shooter (Up)";
        case SHOOTERDOWN: return "Shooter (Down)";
        case SHOOTERLEFT: return "Shooter (Left)";
        case SHOOTERRIGHT: return "Shooter (Right)";
        case MIMROCK: return "Mimrock";
        case CLOUD: return "Cloud";
    }
    return "";
}

/* ------------------------------------------------------------------------- *
 * static int getIcon(int type, int dir)                                     *
 *   type: type of enemy                                                     *
 *   dir : direction of enemy in icon                                        *
 *                                                                           *
 * Returns a tile number as an icon for the specified enemy type. (TED)      *
 * ------------------------------------------------------------------------- */
int Enemy::getIcon(int type, int dir)
{
    switch (type)
    {   case SLUG: return Slug::icon(dir);
        case BOUNDER: return Bounder::icon(dir);
        case MUSHROOM: return Mushroom::icon(dir);
        case LICK: return Lick::icon(dir);
        case ARACHNUT: return Arachnut::icon(dir);
        case WORM: return Worm::icon(dir);
        case SNAKE: return Snake::icon(dir);
        case ANCIENT: return Ancient::icon(dir);
        case SPEARUP: return SpearUp::icon(dir);
        case SPEARDOWN: return SpearDown::icon(dir);
        case SPEARLEFT: return SpearLeft::icon(dir);
        case SPEARRIGHT: return SpearRight::icon(dir);
        case SHOOTERUP: return ShooterUp::icon(dir);
        case SHOOTERDOWN: return ShooterDown::icon(dir);
        case SHOOTERLEFT: return ShooterLeft::icon(dir);
        case SHOOTERRIGHT: return ShooterRight::icon(dir);
        case MIMROCK: return MimRock::icon(dir);
        case CLOUD: return Cloud::icon(dir);
        default: return 0;
    }
}

/* ------------------------------------------------------------------------- *
 * Enemy(...)                                                                *
 *                                                                           *
 * Default constructor for enemies.                                          *
 * ------------------------------------------------------------------------- */
Enemy::Enemy(int type, float x, float y, int xdir, int action, int actiontime,
    int status, bool onground, int frameslot, int xsize, int ysize,
    float xmax, float ymax, float gravity)
:_type(type),_x(x),_y(y),_xdir(xdir),_action(action),_actiontime(actiontime),
 _status(status),_statustime(0),_onground(false),_onslope(false),
 _xsize(xsize),_ysize(ysize),_xmax(xmax),_ymax(ymax),_gravity(gravity),
 _xv(0),_yv(0),_starsprite(-1)
{
    _id = NextID; NextID++;
    _frame = SpriteFrame[frameslot].tile;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
    updateClipPoints();
}

/* ------------------------------------------------------------------------- *
 * float top()                                                               *
 *                                                                           *
 * Returns the Y value of the top of the enemy.                              *
 * ------------------------------------------------------------------------- */
float Enemy::top()
{   return _y - _ysize/2; }

/* ------------------------------------------------------------------------- *
 * void draw(float zoom, int vpx, int vpy)                                   *
 *   zoom: level of zoom (ie: 2 = half size, .5 = double size)               *
 *   vpx,vpy: viewport coordinates ( also used by TED)                       *
 *                                                                           *
 * Draws the enemy if it is on the screen.                                   *
 * ------------------------------------------------------------------------- */
void Enemy::draw(float zoom, int vpx, int vpy)
{
    float x1 = _x - (_txsize/zoom / 2);
    float y1 = _y - (_tysize/zoom / 2);
    float x2 = _x + (_txsize/zoom / 2);
    float y2 = _y + (_tysize/zoom / 2);

    if (onCamera(x1, y1, x2, y2))
    {
        int x = (int)((_x/zoom - getCameraOrigX()) - _txsize/zoom / 2);
        int y = (int)((_y/zoom - getCameraOrigY()) - _tysize/zoom / 2);
        if (zoom == 1) drawSprite(_frame, x+vpx, y+vpy);
        else drawSprite(_frame, x+vpx, y+vpy, zoom);
    }
}

/* ------------------------------------------------------------------------- *
 * int collision(int x1, int y1, int x2, int y2)                             *
 *                                                                           *
 * If a collision with enemy detected, returns enemy's collision type        *
 * (ie: NPCENEMY, DEADLYENEMY, ENDLEVELENEMY) or NOTANENEMY if no collision. *
 * ------------------------------------------------------------------------- */
int Enemy::collision(int x1, int y1, int x2, int y2)
{
    int ex1 = (int)(_x - (_xsize / 2)); int ey1 = (int)(_y - (_ysize / 2));
    int ex2 = (int)(_x + (_xsize / 2)); int ey2 = (int)(_y + (_ysize / 2));

    if (ex1 >= x1 && ex1 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex1 >= x1 && ex1 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    return NOTANENEMY;
}

/* ------------------------------------------------------------------------- *
 * bool collisionb(int x1, int y1, int x2, int y2)                           *
 *                                                                           *
 * If a collision with enemy is detected, returns true. This is independent  *
 * of enemy's type or status, unlike int collision(). Also the frame size is *
 * used for collision instead of clipping size.                              *
 * ------------------------------------------------------------------------- */
bool Enemy::collisionb(int x1, int y1, int x2, int y2)
{
    int ex1 = (int)(_x - (_txsize / 2)); int ey1 = (int)(_y - (_tysize / 2));
    int ex2 = (int)(_x + (_txsize / 2)); int ey2 = (int)(_y + (_tysize / 2));

    if (ex1 >= x1 && ex1 <= x2 && ey1 >= y1 && ey1 <= y2) return true;
    if (ex2 >= x1 && ex2 <= x2 && ey1 >= y1 && ey1 <= y2) return true;
    if (ex1 >= x1 && ex1 <= x2 && ey2 >= y1 && ey2 <= y2) return true;
    if (ex2 >= x1 && ex2 <= x2 && ey2 >= y1 && ey2 <= y2) return true;
    if (x1 >= ex1 && x1 <= ex2 && y1 >= ey1 && y1 <= ey2) return true;
    if (x2 >= ex1 && x2 <= ex2 && y1 >= ey1 && y1 <= ey2) return true;
    if (x1 >= ex1 && x1 <= ex2 && y2 >= ey1 && y2 <= ey2) return true;
    if (x2 >= ex1 && x2 <= ex2 && y2 >= ey1 && y2 <= ey2) return true;
    return false;
}

/* ------------------------------------------------------------------------- *
 * void tick()                                                               *
 *                                                                           *
 * Handles the enemy's physics, AI and animation frames for one game cycle.  *
 * ------------------------------------------------------------------------- */
void Enemy::tick()
{
    if (inRange(320))
    {   collideWithShots();
        doPhysics();
        doAI();
        updateFrame();
    }
}

/* ------------------------------------------------------------------------- *
 * void collideWithShots()                                                   *
 *                                                                           *
 * Checks for collisions with any player shots and stuns player if true.     *
 * ------------------------------------------------------------------------- */
void Enemy::collideWithShots()
{   if (_status != ENEMYDEAD &&
        Shot::collideShot((int)(_x-_xsize/2), (int)(_y-_ysize/2),
        (int)(_x+_xsize/2), (int)(_y+_ysize/2), PLAYERSHOT)) kill();
}

/* ------------------------------------------------------------------------- *
 * void kill()                                                               *
 *                                                                           *
 * Stuns the enemy and creates a stars sprite above its head.                *
 * ------------------------------------------------------------------------- */
void Enemy::kill()
{
    _status = ENEMYDEAD; _action = ENEMYDEAD;
    _actiontime = 0;
    _yv -= 6;
    _xv *= 4;
    
    // --- add stars to spite collection ---
    _starsprite = Sprite::addSprite((int)_x, (int)(_y-_ysize/2), STARS_START,
        STARS_END, 6, 22, 15, false, true, -1);
}

/* ------------------------------------------------------------------------- *
 * bool stoodUpon(float x, float y)                                          *
 *   x,y: position of feet to check                                          *
 *                                                                           *
 * Returns true if the enemy is being stood upon at the given location.      *
 * ------------------------------------------------------------------------- */
bool Enemy::stoodUpon(float x, float y)
{   return false; /* Only implemented by bounder */ }

/* ------------------------------------------------------------------------- *
 * void doPhysics()                                                          *
 *                                                                           *
 * Handles physics for the enemy. Base method which may be overwritten by an *
 * inheriting class.                                                         *
 * ------------------------------------------------------------------------- */
void Enemy::doPhysics()
{
    // save location for updating starsprite's location
    float oldx = _x, oldy = _y;
    
    updateClipPoints();
    
    // ---- Gravity ----
    if (!_onground) _yv += _gravity;
    
    // ---- Max Speed ----
    if (_xv > _xmax) _xv = _xmax;
    if (_yv > _ymax) _yv = _ymax;
    if (_xv < -_xmax) _xv = -_xmax;
    if (_yv < -_ymax) _yv = -_ymax;

    // ---- Move Enemy ----
    _x += _xv;
    _y += _yv;
    
    // ---- Friction ----
    _xv *= Friction / 2;
    
    // ---- Zero Horizontal Velocity ----
    if (_xv < .2 && _xv > 0) _xv = 0;
    if (_xv > -.2 && _xv < 0) _xv = 0;
    
    // ---- Map Boundaries ----
    if (_x < 0) { _x = 0; _xv = 0; }
    if (_y < 0) { _y = 0; _yv = 0; }
    if (_x > mapWidth() * ts - 1) { _x = mapWidth() * ts - 1; _xv = 0; }
    if (_y > mapHeight() * ts - 1) { _y = mapHeight() * ts - 1; _yv = 0; }

    // - - - - - - - - - - - - - - Clipping - - - - - - - - - - - - - -
    /* +--1--+
       |     |
       3     4
       |     |
       +--2--+ */

    // If not on ground, start falling
    doOnGround();
    
    // ---- Clip to walls and slopes ----
    updateClipPoints();
    for (int c=1; c<=4; c++)
    {   switch(mapHitPoint(_clippoint[c].x, _clippoint[c].y))
        {
            case WALL: hitWall(c, _clippoint[c].x, _clippoint[c].y); break;
            case SLOPE:
            if (c==2) hitSlope(_clippoint[c].x, _clippoint[c].y, 0); break;
            case HAZZARD: hitWall(c, _clippoint[c].x, _clippoint[c].y); break;
            case ONEWAYFLOOR:
            if (c==2&&_yv>0) hitWall(c,_clippoint[c].x,_clippoint[c].y); break;
            case FGSLOPE:
            if (c==2) hitSlope(_clippoint[c].x, _clippoint[c].y, 1); break;
            case FGWALL: hitWall(c, _clippoint[c].x, _clippoint[c].y); break;
        }
    }
    updateClipPoints();

    // ---- If enemy moved, move stars with it ----
    if (_status == ENEMYDEAD)
    {    Sprite::getSpriteByID(_starsprite)->move(_x-oldx, _y-oldy);
    }
    
    // ---- if player standing on this enemy ----
    if (getPlayerOnBall() == _id)
    {   setPlayerLoc(getPlayerX() + (_x-oldx), getPlayerY() + (_y-oldy)); }
}

/* ------------------------------------------------------------------------- *
 * void doOnGround()                                                         *
 *                                                                           *
 * Checks what the enemy is standing on and updates the _onground and        *
 * _onslope variables accordingly.                                           *
 * ------------------------------------------------------------------------- */
void Enemy::doOnGround()
{
    // --- previous ground values ---
    bool oldonslope = _onslope;
    bool oldonground = _onground;
    // --- map location of enemy's feet ---
    int mapx = (int)(_x / ts) + 1;
    int mapy = (int)((_y + (_ysize / 2)) / ts) + 1;

    // - - - - - - - - - - CHECK FOR GROUND TILE - - - - - - - - - -
    if (_yv >= 0)
    {
        if ((int)(_y+(_ysize/2))%ts >= ts-1)
        {   mapx = (int)(_x/ts)+1; mapy = (int)((_y+(_ysize/2))/ts)+2;
            if (tileType(getMap(mapx, mapy)) == WALL || 
                tileType(getMapFG(mapx, mapy)) == FGWALL ||
                tileType(getMapFG(mapx, mapy)) == ONEWAYFLOOR)
            {   _onground = true; _onslope = false;
                if ((int)(_y+(_ysize/2))%ts < ts-1) _y = (int)_y + 1;
            }
            else
            {   _onground = false; _onslope = false; }
        }
        else
        {   _onground = false; _onslope = false; }
    }
    else
    {   _onground = false; _onslope = false; }

    // - - - - - - - - - - - CHECK FOR SLOPES - - - - - - - - - - -
    if (tileType(getMap(mapx, mapy)) == SLOPE ||
        tileType(getMapFG(mapx, mapy)) == SLOPE)
    {
        int layer;
        float tmpx=_x, tmpy=_y+(_ysize/2);
        if (tileType(getMap(mapx, mapy)) == SLOPE) layer = 0; else layer = 1;
        int h1 = getSlopeHeight((int)tmpx, (int)tmpy, layer);
        int h2 = getSlopeHeight((int)tmpx, (int)tmpy+1, layer);
        if (h1 == 0 && h2 < 0)
        {   _onground = true; _onslope = true; }
        else
        {   _onground = false; _onslope = false; }
    }
    
    // - - - - - - - - - - KEEP PLAYER ON A SLOPE - - - - - - - - - -
    if (!_onground && oldonground && _yv >= 0)
    {   bool sticktoslope=false;
        mapx=(int)(_x/ts)+1; mapy=(int)((_y+(_ysize/2))/ts)+1;
        if (tileType(getMap(mapx, mapy)) == SLOPE ||
            tileType(getMapFG(mapx, mapy)) == SLOPE)
        {   sticktoslope=true; }
        else if (tileType(getMap(mapx, mapy+1)) == SLOPE ||
                 tileType(getMapFG(mapx, mapy+1)) == SLOPE)
        {   sticktoslope=true; mapy++;
            _y += (ts - ((int)(_y + (_ysize / 2) + ts) % ts));
        }
        else if (tileType(getMap(mapx, mapy+1)) == WALL ||
                 tileType(getMapFG(mapx, mapy+1)) == FGWALL)
        {   _y += (ts - (int)(_y + (_ysize / 2) + ts) % ts) - 1;
            _yv = 0; _onground = true;
        }

        if (sticktoslope)
        {   int layer;
            if (tileType(getMap(mapx, mapy)) == SLOPE) layer = 0;
            else layer = 1;
            _y += (ts - (int)(_y + (_ysize / 2) + 1) % ts) - 1;
            _y += getSlopeHeight((int)_x, (int)(_y+(_ysize/2)+1), layer);
            _yv = 0; _onground = true; _onslope = true;
        }
    }
}

/* ------------------------------------------------------------------------- *
 * void hitWall(int p, float hx, float hy)                                   *
 *   p    : clippoint that hit the tile                                      *
 *   hx,hy: location of collision                                            *
 *                                                                           *
 * Performs clipping between player and a single wall tile. Uses the players *
 * hitpoint colliding with the tile to move him back outside of it again.    *
 * ------------------------------------------------------------------------- */
void Enemy::hitWall(int p, float hx, float hy)
{
    int side = -1;
    float nx, ny;
    if (p==1) { ny = (int)((hy - ((int)hy % ts)) + ts); side = UP; }
    else if (p==2) { ny = (int)((hy - ((int)hy % ts)) - 1); side = DOWN; }
    else if (p==3) { nx = (int)((hx - ((int)hx % ts)) + ts); side = RIGHT; }
    else if (p==4) { nx = (int)((hx - ((int)hx % ts)) - 1); side = LEFT; }
    
    switch (side)
    {   case UP: _yv = 0; _y = ny + (_ysize / 2) + 1; break;
        case DOWN: _yv = 0; _y = ny - (_ysize / 2); _onground = true; break;
        case LEFT: _xv = 0; _x = nx - (_xsize / 2); break;
        case RIGHT: _xv = 0; _x = nx + (_xsize / 2); break;
    }

    updateClipPoints();
}

/* ------------------------------------------------------------------------- *
 * void hitSlope(float x, float y, int l)                                    *
 *   x,y: location of clippoint collision                                    *
 *   l  : layer of collision (0=background, 1=foreground)                    *
 *                                                                           *
 * Performs clipping between the enemy and a single slope tile. Moves the    *
 * enemy by his bottom (_clippoint[2]) up to stand on the slope.             *
 * ------------------------------------------------------------------------- */
void Enemy::hitSlope(float x, float y, int l)
{
    int h = getSlopeHeight((int)x, (int)y, l);
    if (h > ts) h = ts;
    if (h < -ts) h - -ts;

    if (h != 0)
    {
        _y += h;
        _yv = 0;
        _onground = true;
        _onslope = true;
        updateClipPoints();
    }
}

/* ------------------------------------------------------------------------- *
 * bool holeHere(int mx, int my)                                             *
 *   mx,my: location to check (in tile units)                                *
 *                                                                           *
 * Returns true if there is a hole at this point on the map. mx, my is in    *
 * tile units.                                                               *
 * ------------------------------------------------------------------------- */
bool Enemy::holeHere(int mx, int my)
{
    int bg = tileType(getMap(mx, my));
    int fg = tileType(getMapFG(mx, my));
    
    if ((bg == PASSIVE && fg == PASSIVE) || bg == HAZZARD || fg == HAZZARD)
    {   return true; }
    return false;
}

/* ------------------------------------------------------------------------- *
 * void dropToGround()                                                       *
 *                                                                           *
 * Drops this enemy to the ground, or the bottom of the map if no ground is  *
 * below it. It works like a modified doPhysics() method for dropping the    *
 * enemy until it collides with the map. Used for TED, called by createEnemy.*
 * ------------------------------------------------------------------------- */
void Enemy::dropToGround()
{
    while ((!_onground) && (_y < mapHeight() * ts - 1))
    {
        updateClipPoints();
    
        // ---- Gravity ----
        if (!_onground) _yv += 1;

        // ---- Max Speed ----
        if (_yv > 5) _yv = 5;
        if (_yv < -5) _yv = -5;

        // ---- Move Enemy ----
        _y += _yv;

        // ---- Map Boundaries ----
        if (_y < 0) { _y = 0; _yv = 0; }
        if (_y > mapHeight() * ts - 1) { _y = mapHeight() * ts - 1; _yv = 0; }

        // - - - - - - - - - - - - - - Clipping - - - - - - - - - - - - - -
        // If not on ground, start falling
        doOnGround();
    
        // ---- Clip to walls and slopes ----
        updateClipPoints();
        for (int c=1; c<=4; c++)
        {   switch(mapHitPoint(_clippoint[c].x, _clippoint[c].y))
            {
                case WALL:
                    hitWall(c, _clippoint[c].x, _clippoint[c].y);
                    break;
                case SLOPE:
                    if (c==2) hitSlope(_clippoint[c].x, _clippoint[c].y, 0);
                    break;
                case HAZZARD:
                    hitWall(c, _clippoint[c].x, _clippoint[c].y);
                    break;
                case ONEWAYFLOOR:
                    if (c==2&&_yv>0)
                        hitWall(c,_clippoint[c].x,_clippoint[c].y);
                    break;
                case FGSLOPE:
                    if (c==2) hitSlope(_clippoint[c].x, _clippoint[c].y, 1);
                    break;
                case FGWALL:
                    hitWall(c, _clippoint[c].x, _clippoint[c].y);
                    break;
            }
        }
        updateClipPoints();
    }
}

/* ------------------------------------------------------------------------- *
 * bool inRange(int dist)                                                    *
 *   dist: distance (pixels)                                                 *
 *                                                                           *
 * Returns true if this enemy is within 'dist' pixels of player.             *
 * ------------------------------------------------------------------------- */
bool Enemy::inRange(int dist)
{
    if (abs((int)(getPlayerX() - _x)) <= dist &&
        abs((int)(getPlayerY() - _y)) <= dist)
    {   return true; }
    return false;
}

/* ------------------------------------------------------------------------- *
 * bool inRangeY(int dist)                                                   *
 *   dist: distance (pixels)                                                 *
 *                                                                           *
 * Returns true if this enemy is within 'dist' pixels on the player in the   *
 * Y direction.                                                              *
 * ------------------------------------------------------------------------- */
bool Enemy::inRangeY(int dist)
{   if (abs((int)(getPlayerY() - _y)) <= dist) return true; return false; }

/* ------------------------------------------------------------------------- *
 * bool betweenRangesX(int x1, int x2)                                       *
 *   x1: near distance                                                       *
 *   x2: far distance                                                        *
 *                                                                           *
 * Returns true if the enemy is at an X distance from player greater than    *
 * the near distance and less than the far distance.                         *
 * ------------------------------------------------------------------------- */
bool Enemy::betweenRangesX(int x1, int x2)
{
    if (abs((int)(getPlayerX()-_x)) >= x1 && abs((int)(getPlayerX()-_x)) <= x2)
        return true;
    return false;
}

/* ------------------------------------------------------------------------- *
 * void updateClipPoints()                                                   *
 *                                                                           *
 * Updates values in the _clippoint array used for clipping.                 *
 * ------------------------------------------------------------------------- */
void Enemy::updateClipPoints()
{
    _clippoint[1].x = _x; _clippoint[1].y = _y - _ysize / 2;
    _clippoint[2].x = _x; _clippoint[2].y = _y + _ysize / 2;
    _clippoint[3].x = _x - _xsize / 2; _clippoint[3].y = _y;
    _clippoint[4].x = _x + _xsize / 2; _clippoint[4].y = _y;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void TEDEnemy::drawEnemies(float zoom, int vpx, int vpy)
{   for (int e=0; e<Enemies.size(); e++) Enemies[e]->draw(zoom, vpx, vpy); }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SpearEnemy::SpearEnemy(int type, float x, float y, int frameslot, int xsize,
    int ysize)
:Enemy(type, x, y, LEFT, 0, maxfps*2, SPEARWAIT, false, frameslot, xsize, ysize,
 1, 1, 1) {}
int SpearEnemy::collision(int x1, int y1, int x2, int y2)
{
    if (_status != SPEARATTACK) return NOTANENEMY;
    int ex1 = (int)(_x - (_xsize / 2)); int ey1 = (int)(_y - (_ysize / 2));
    int ex2 = (int)(_x + (_xsize / 2)); int ey2 = (int)(_y + (_ysize / 2));

    if (ex1 >= x1 && ex1 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex1 >= x1 && ex1 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    return NOTANENEMY;
}
void SpearEnemy::tick()
{   doAI(); updateFrame(); }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

ShooterEnemy::ShooterEnemy(int type, float x, float y, int frameslot, int xsize,
    int ysize)
:Enemy(type, x, y, LEFT, 0, 0, SHOOTERWAIT, false, frameslot, xsize, ysize,
 1, 1, 1) {}
void ShooterEnemy::tick()
{   doAI(); updateFrame(); }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Slug::Slug(float x, float y, int dir)
:Enemy(SLUG, x, y, dir, SLUGCRAWL1, 2, SLUGCRAWLING, true, 0, 16, 24, 16, 16,
 1) {}
void Slug::doAI()
{   
    // dont do ai if dead
    if (_status == ENEMYDEAD) return;
    
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    if (_actiontime <= 0 && _status == SLUGPOOPING)
    {   _status = SLUGCRAWLING; _action = SLUGCRAWL1; }

    // --- handle status ---
    if (_status == SLUGCRAWLING)
    {   // -- continue crawling --
        if (_actiontime <= 0)
        {   if (_action == SLUGCRAWL1)
            {   if (_xdir == LEFT) _xv = -6; else _xv = 6;
                _action = SLUGCRAWL2; _actiontime = maxfps / 5;
            }
            else if (_action == SLUGCRAWL2)
            {   _action = SLUGCRAWL1; _actiontime = maxfps / 5; }
        }
        
        // -- change direction --
        int mx, my;
        bool changedir = false;     // Should slug switch directions?
        if (_xdir == LEFT) mx = (int)(((_x - _xsize / 2) - 6) / ts) + 1;
        if (_xdir == RIGHT) mx = (int)(((_x + _xsize / 2) + 6) / ts) + 1;
        // - due to random change -
        if (rand() % 150 == 1)
        {   changedir = true; }
        // - due to map edge -
        my = (int)(_y / ts) + 1;
        if (tileType(getMap(mx, my)) == WALL || mx > mapWidth() || mx < 0)
        {   changedir = true; }
        // - due to big hole -
        my = (int)((_y + _ysize / 2 - 4) / (float)ts) + 2;
        if (holeHere(mx, my))
        {   changedir = true; }
        if (changedir)
        {   if (_xdir == LEFT) _xdir = RIGHT; else _xdir = LEFT; }
        
        // -- possibility of pooping --
        if (rand() % 350 == 1)
        {
            if (inRange(240)) Audio::playSound(18);
            // - set new status -
            _status = SLUGPOOPING;
            _action = SLUGPOOPING;
            _actiontime = maxfps;
            // - create poop -
            int x = (int)_x; int y = (int)_y + _ysize / 2;
            Sprite::addSprite(x, y, SpriteFrame[8].tile, SpriteFrame[9].tile,
                maxfps * 2, SpriteFrame[8].width, SpriteFrame[8].height,
                true, false, -1);
        }
    }
}
void Slug::updateFrame()
{
    int frameslot;
    switch (_action)
    {   case SLUGCRAWL1:
        if (_xdir == LEFT) frameslot = 0; else frameslot = 3; break;
        case SLUGCRAWL2:
        if (_xdir == LEFT) frameslot = 1; else frameslot = 4; break;
        case SLUGPOOPING:
        if (_xdir == LEFT) frameslot = 2; else frameslot = 5; break;
    }
    
    if (_status == ENEMYDEAD) frameslot = 7;

    _frame = SpriteFrame[frameslot].tile;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Bounder::Bounder(float x, float y)                     // xsize,ysize was 24,24
:Enemy(BOUNDER, x, y, UP, 0, 0, BALLBOUNCEUP, false, 10, 24, 21, 4, 6, .5) {}
int Bounder::collision(int x1, int y1, int x2, int y2) { return NOTANENEMY; }
bool Bounder::stoodUpon(float x, float y)
{
    float ex1 = _x - _txsize / 2, ey1 = _y - _tysize / 2;
    float ex2 = _x + _txsize / 2, ey2 = _y - 1;
    
    if (x >= ex1 && x <= ex2 && y >= ey1 && y <= ey2) return true;
    return false;        
}
void Bounder::doAI()
{
    if (_status == ENEMYDEAD) return;
    
    int newdir;
    // --- direction to chase player ---
    if (getPlayerX() > _x) newdir = RIGHT; else newdir = LEFT;
    
    // --- decrease timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- movement ---
    if (_actiontime > 0) _yv -= 2;
    if (!_onground)
    {   if (_xdir == UP) _xv = 0;
        if (_xdir == LEFT) _xv -= 2;
        if (_xdir == RIGHT) _xv += 2;
    }
    
    // --- bounce on ground ---
    if (_onground && _actiontime == 0)
    {   _actiontime = 5;
        _action -= 1;
        if (_action <= 0)
        {   if (_status == BALLBOUNCEUP)
            {   _status = BALLFOLLOW;
                _action = rand() % 3;
                if (rand() % 3 == 0)
                {   if (newdir == LEFT) _xdir = RIGHT; else _xdir = LEFT; }
                else
                {   _xdir = newdir; }
            }
            else
            {   _status = BALLBOUNCEUP;
                _action = (rand() % 3) + 1;
                _xdir = UP;
                _xv = 0;
            }
        }
    }
}
void Bounder::updateFrame()
{
    int frameslot;                                   /*  \/ animation delay */
    if (_xdir == LEFT) frameslot = 10 + ((anmCounter() / 2) % 2);
    else if (_xdir == RIGHT) frameslot = 12 + ((anmCounter() / 2) % 2);
    else if (_xdir == UP) frameslot = 14 + ((anmCounter() / 2) % 2);

    if (_status == ENEMYDEAD) frameslot = 16;
    
    _frame = SpriteFrame[frameslot].tile;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Mushroom::Mushroom(float x, float y)
:Enemy(MUSHROOM, x, y, LEFT, 0, 0, MUSHBOUNCEHI, true, 17, 32, 32, 0, 5, .5) {}
void Mushroom::collideWithShots()
{   Shot::collideShot((int)(_x-_xsize/2), (int)(_y-_ysize/2),
        (int)(_x+_xsize/2), (int)(_y+_ysize/2), PLAYERSHOT);
}
void Mushroom::doAI()
{
    // --- Update direction ---
    if (getPlayerX() > _x) _xdir = RIGHT; else _xdir = LEFT;
    
    // --- Decrease timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- Jump up if in action ---
    if (_actiontime > 0) _yv -= 2;
    
    // --- Bounce on the ground ---
    if (_onground && _actiontime == 0)
    {   if (_status == MUSHBOUNCEHI)
        {   // -- switch to low bounce --
            if (inRange(300)) Audio::playSound(20);
            _status = MUSHBOUNCELO;
            _action = 1;
            _actiontime = 4;        // ticks to jump
        }
        else if (_status == MUSHBOUNCELO)
        {   if (inRange(300)) Audio::playSound(20);
            _action++;
            if (_action > 2)
            {   // -- switch to hi bounce --
                _status = MUSHBOUNCEHI;
                _action = 0;
                _actiontime = 12;   // ticks to jump
            }
            else
            //  -- continue low bounce --
            {   _actiontime = 4; }
        }
    }
}
void Mushroom::updateFrame()
{
    int frameslot;
    if (_xdir == LEFT) frameslot = 17 + (anmCounter() / 2) % 2;
    if (_xdir == RIGHT) frameslot = 19 + (anmCounter() / 2) % 2;
    
    _frame = SpriteFrame[frameslot].tile;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Lick::Lick(float x, float y, int dir)
:Enemy(LICK, x, y, dir, 0, 0, LICKSIT, false, 21, 22, 16, 4, 6, 1) {}
int Lick::collision(int x1, int y1, int x2, int y2)
{
    if (_status != LICKATTACK) return NOTANENEMY;
    int ex1 = (int)(_x - (_xsize / 2)); int ey1 = (int)(_y - (_ysize / 2));
    int ex2 = (int)(_x + (_xsize / 2)); int ey2 = (int)(_y + (_ysize / 2));

    if (ex1 >= x1 && ex1 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex1 >= x1 && ex1 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    return NOTANENEMY;
}
void Lick::doAI()
{   /*
    if sitonground
      if actiontime = 0 then choose new action
        if decided to attack, status = attack
        if decided to bounce
          status = bounce
          move player up
    if bouncefollow
      if in the air then move
      if on the ground reset to sitonground
    if attacking */

    if (_status == ENEMYDEAD) return;
    
    // --- direction to chase player ---
    int newdir;
    if (getPlayerX() > _x) newdir = RIGHT; else newdir = LEFT;

    if (_actiontime > 0) _actiontime--;
    
    // --- movement ---
    if (_status == LICKSIT)
    {   if (_action == 0)
        {   if (inRange(24) && getPlayerAction() != DYING)
            {   _xdir = newdir;
                _status = LICKATTACK;
                _action = LICKFIRESML;
                _actiontime = 6;
                _xsize = 26;
            }
            else
            {   _xdir = newdir;
                _status = LICKFOLLOW;  /* \/ little hop       \/ big hop */
                if (inRange(36)) _actiontime = 3; else _actiontime = 5;
            }
        }
        else { _action--; }
    }
    else if (_status == LICKFOLLOW)
    {   if (_actiontime > 0) _yv -= 2;
        if (!_onground)
        {   if (_xdir == LEFT) _xv -= 2; else _xv += 2; }
        else if (_onground && _actiontime == 0)
        {   _status = LICKSIT;
            _action = 3;        // sit on ground 3 ticks
            _xv = 0;
        }
    }
    else if (_status == LICKATTACK)
    {   if (_actiontime == 0)
        {   if (_action == LICKFIRESML)
            {   _action = LICKFIREMED;
                _actiontime = 6;
                _xsize = 38;
            }
            else if (_action == LICKFIREMED)
            {   _action = LICKFIREBIG;
                _actiontime = 6;
                _xsize = 40;
            }
            else if (_action == LICKFIREBIG)
            {   _status = LICKSIT;
                _action = 3;
                _xsize = 22;
                if (_xdir == LEFT) _x += 9; else _xdir -= 9;  // kick-back
            }                                                 // from flames
        }
    }
}
void Lick::updateFrame()
{
    int frameslot, anmoffset = 0;
    if (_status == LICKATTACK)
    {   if (_xdir == LEFT) frameslot = 24 + (_action - 1);
        else frameslot = 27 + (_action - 1);
    }
    else if (_status == ENEMYDEAD)
    {   frameslot = 23; }
    else
    {   if (_xdir == LEFT) frameslot = 21; else frameslot = 22;
        if (_onground)
        {   if (_action > 0) anmoffset = 3; else anmoffset = 0; }
        else
        {   if (_yv >= 0) anmoffset = 1; else anmoffset = 2; }
    }
    
    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Arachnut::Arachnut(float x, float y, int dir)
:Enemy(ARACHNUT, x, y, dir, 0, 0, ARACHCRAWLING, false, 30, 40, 40, 4, 6, 1)
{ }
int Arachnut::collision(int x1, int y1, int x2, int y2)
{
    if (_status != ARACHCRAWLING) return NOTANENEMY;
    int ex1 = (int)(_x - (_xsize / 2)); int ey1 = (int)(_y - (_ysize / 2));
    int ex2 = (int)(_x + (_xsize / 2)); int ey2 = (int)(_y + (_ysize / 2));

    if (ex1 >= x1 && ex1 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex1 >= x1 && ex1 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    return NOTANENEMY;
}
void Arachnut::kill()
{   _status = ARACHSTUNNED; _actiontime = maxfps * 5; _xv = 0; }
void Arachnut::doAI()
{
    if (_status == ENEMYDEAD) return;
    
    if (_actiontime > 0) _actiontime--;
    
    // --- handle status ---
    if (_status == ARACHCRAWLING)
    {   if (_xdir == LEFT) _xv -= _xmax / 2; else _xv += _xmax / 2;
        
        // -- change direction --
        bool changedir = false;
        int mx, my;
        if (_xdir == LEFT) mx = (int)(((_x - _xsize / 2) - 8) / ts) + 1;
        else mx = (int)(((_x + _xsize / 2) + 8) / ts) + 1;
        // - random change -
        if (rand() % 150 == 0) changedir = true;
        // - wall or map edge -
        my = (int)(_y / ts) + 1;
        if (tileType(getMap(mx, my)) == WALL ||
            mx > mapWidth() || mx < 0) changedir = true;
        // - big hole -
        my = (int)((_y + (_ysize / 2)) / ts) + 2;
        if (holeHere(mx, my)) changedir = true;
        // - do the change -
        if (changedir)
        {   if (_xdir == LEFT) { _xdir = RIGHT; _xv = _xmax / 4; }
            else { _xdir = LEFT; _xv = -_xmax / 4; }
        }
    }
    else if (_status == ARACHSTUNNED)
    {   if (_actiontime == 0)                          // 4 is animation delay
        {   _actiontime = (int)((maxfps * 2) / 4) * 4; // so actiontime should
            _status = ARACHWAKING;                     // be a multiple of 4
        }
    }
    else if (_status == ARACHWAKING)
    {   if (_actiontime == 0) _status = ARACHCRAWLING; }
}
void Arachnut::updateFrame()
{
    int frameslot, anmoffset = 0;
    if (_status == ARACHCRAWLING)
    {   if (_xdir == LEFT)
        { frameslot = 30; anmoffset = 3 - anmCounter() % 4; }
        else
        { frameslot = 30; anmoffset = anmCounter() % 4; }
    }
    else if (_status == ARACHWAKING)
    {   frameslot = 30 + (int)(_actiontime / 8) % 2; }
    else if (_status == ARACHSTUNNED)
    {   frameslot = 31; }
    
    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Worm::Worm(float x, float y, int dir)
:Enemy(WORM, x, y, dir, 0, 0, WORMCRAWL1, true, 32, 24, 16,
 6 + (rand()/RAND_MAX) * 2, 4, 1) {}
int Worm::collision(int x1, int y1, int x2, int y2) { return NOTANENEMY; }
void Worm::collideWithShots() {}
void Worm::doAI()
{
    // Chase the player
    if (getPlayerX() > _x) _xdir = RIGHT; else _xdir = LEFT;
    
    // Handle action timer
    if (_actiontime > 0) _actiontime--;
    
    // -- Forced change direction --
    bool changedir=false;
    int mx, my;
    if (_xdir == LEFT) mx = (int)(((_x - _xsize / 2) - 4) / ts) + 1;
    else mx = (int)(((_x + _xsize / 2) + 4) / ts) + 1;
    // - wall or map edge -
    my = (int)(_y / ts) + 1;
    if (tileType(getMap(mx, my)) == WALL || mx > mapWidth() || mx < 0)
    {   changedir = true; }
    // - big hole -
    my = (int)(_y / ts) + 2;
    if (holeHere(mx, my)) changedir = true;
    // - change the direction -
    if (changedir) { if (_xdir == LEFT) _xdir = RIGHT; else _xdir = LEFT; }

    // -- Crawling --
    if (_actiontime <= 0)
    {   if (_status == WORMCRAWL1)
        {   if (_xdir == LEFT) _xv = -_xmax; else _xv = _xmax;
            _status = WORMCRAWL2; _actiontime = maxfps / 2; 
        }
        else if (_status == WORMCRAWL2)
        {   _status = WORMCRAWL1; _actiontime = maxfps / 2; }
    }
}
void Worm::updateFrame()
{
    int frameslot, anmoffset = 0;
    if (_xdir == LEFT) frameslot = 32; else frameslot = 33;
    if (_status == WORMCRAWL1) anmoffset = 1;

    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Snake::Snake(float x, float y, int dir)
:Enemy(SNAKE, x, y, dir, 0, 0, SNAKEMOVE, true, 34, 16, 24, 3, 3, 1) {}
int Snake::collision(int x1, int y1, int x2, int y2)
{
    if (_status != SNAKEATTACK || _action != 2) return NOTANENEMY;
    int ex1 = (int)(_x - (_xsize / 2)); int ey1 = (int)(_y - (_ysize / 2));
    int ex2 = (int)(_x + (_xsize / 2)); int ey2 = (int)(_y + (_ysize / 2));
    
    if (ex1 >= x1 && ex1 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex1 >= x1 && ex1 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    return NOTANENEMY;
}
void Snake::collideWithShots()
{   if (_status == SNAKEATTACK &&
        Shot::collideShot((int)(_x-_xsize/2), (int)(_y-_ysize/2),
            (int)(_x+_xsize/2), (int)(_y+_ysize/2), PLAYERSHOT)) kill();
}
void Snake::doAI()
{
    if (_status == ENEMYDEAD) return;
    
    // --- Handle action timer ---
    if (_actiontime > 0) _actiontime--;

    // --- Handle status ---
    if (_status == SNAKEMOVE)
    {   if (_xdir == LEFT) _xv -= _xmax / 2; else _xv += _xmax / 2;
        
        // -- walls and holes --
        bool changedir=false;
        int mx, my;
        if (_xdir == LEFT) mx = (int)(((_x - _xsize / 2) - 8) / ts) + 1;
        else mx = (int)(((_x + _xsize / 2) + 8) / ts) + 1;
        // - wall or map edge -
        my = (int)(_y / ts) + 1;
        if (tileType(getMap(mx, my)) == WALL || mx > mapWidth() || mx < 0)
        {   changedir = true; }
        // - big hole -
        my = (int)((_y + (_ysize / 2)) / ts) + 2;
        if (holeHere(mx, my)) changedir = true;
        // - change the direction -
        if (changedir)
        {   if (_xdir == LEFT) { _xdir = RIGHT; _xv = _xmax / 4; }
            else { _xdir = LEFT; _xv = -_xmax / 4; }
        }
        
        // -- Randomly peek for player --
        if (rand() % 150 == 0)
        {   _status = SNAKEPEEK;                    // looks up to find player
            _action = _xdir;
            _actiontime = maxfps;
        }
        
        // --- Attach player ---
        if (abs((int)(getPlayerX() - _x)) < 36 && _y > getPlayerY() &&
            _y - getPlayerY() < 12)
        {   if ((_xdir == LEFT && getPlayerX() < _x) ||
                (_xdir == RIGHT && getPlayerX() > _x) &&
                getPlayerAction() != DYING)
            {   _status = SNAKEATTACK;
                _action = 0;
                _actiontime = maxfps / 10;
            }
        }
    }
    else if (_status == SNAKEPEEK)
    {   if (_actiontime == 0)
        {   if (_xdir == _action)                   // --- first peek ---
            {   if (_xdir == LEFT)
                {   if (getPlayerX() < _x)
                    {   _status = SNAKEMOVE;        // found player to left
                        _action = 0;
                    }
                    else
                    {   _xdir = RIGHT;              // not left, look right
                        _actiontime = maxfps;
                    }
                }
                else /* (_xdir == RIGHT) */
                {   if (getPlayerX() > _x)
                    {   _status = SNAKEMOVE;        // found player to right
                        _action = 0;
                    }
                    else
                    {   _xdir = LEFT;               // not right, look left
                        _actiontime = maxfps;
                    }
                }
            }
            else                                    // --- second peek ---
            {   _status = SNAKEMOVE;
                _action = 0;
            }
        }
    }
    else if (_status == SNAKEATTACK)
    {   if (_actiontime == 0)
        {   switch (_action)
            {   case 0: _action = 1; _actiontime = maxfps / 8; break;
                case 1: _action = 2; _actiontime = maxfps / 3;
                        _xsize = SpriteFrame[38].width; break;
                case 2: _action = 3; _actiontime = maxfps / 8;
                        _xsize = 16; break;
                case 3: _status = SNAKEMOVE; _action = 0; break;
            }
        }
    }
}
void Snake::updateFrame()
{
    int frameslot, anmoffset = 0;
    if (_status == SNAKEMOVE)
    {   frameslot = 34; }
    else if (_status == SNAKEPEEK)
    {   if (_xdir == LEFT) frameslot = 35; else frameslot = 36;
        if (_actiontime  < (maxfps * 2) / 3 && _actiontime > maxfps / 3)
        {   anmoffset = 1; }
    }
    else if (_status == SNAKEATTACK)
    {   if (_action == 2)
        {   if (_xdir == LEFT) frameslot = 38; else frameslot = 40; }
        else if (_action == 3)
        {   if (_xdir == LEFT) frameslot = 37; else frameslot = 39;
            anmoffset = 1;
        }
        else
        {   if (_xdir == LEFT) frameslot = 37; else frameslot = 39;
            anmoffset = _action;
        }
    }
    else if (_status == ENEMYDEAD)
    {   frameslot = 41; }
    
    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Ancient::Ancient(float x, float y, int dir)
:Enemy(ANCIENT, x, y, dir, OLDMANWALK1, 2, OLDMANWALKING, true, 42, 16, 38,
 16, 16, 1) {}
int Ancient::collision(int x1, int y1, int x2, int y2)
{
    int ex1 = (int)(_x - (_xsize / 2)); int ey1 = (int)(_y - (_ysize / 2));
    int ex2 = (int)(_x + (_xsize / 2)); int ey2 = (int)(_y + (_ysize / 2));

    if (ex1 >= x1 && ex1 <= x2 && ey1 >= y1 && ey1 <= y2) return ENDLEVELENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey1 >= y1 && ey1 <= y2) return ENDLEVELENEMY;
    if (ex1 >= x1 && ex1 <= x2 && ey2 >= y1 && ey2 <= y2) return ENDLEVELENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey2 >= y1 && ey2 <= y2) return ENDLEVELENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y1 >= ey1 && y1 <= ey2) return ENDLEVELENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y1 >= ey1 && y1 <= ey2) return ENDLEVELENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y2 >= ey1 && y2 <= ey2) return ENDLEVELENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y2 >= ey1 && y2 <= ey2) return ENDLEVELENEMY;
    return NOTANENEMY;
}
void Ancient::collideWithShots() {}
void Ancient::doAI()
{
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    // -- reset to walking --
    if (_status == OLDMANBROODING && _actiontime <= 0)
    {   _status = OLDMANWALKING; _action = OLDMANWALK1; }
    
    // --- handle status ---
    if (_status == OLDMANWALKING)
    {   // -- continue walking --
        if (_actiontime <= 0)
        {   if (_action == OLDMANWALK1)
            {   if (_xdir == LEFT) _xv = -6; else _xv = 6;
                _action = OLDMANWALK2; _actiontime = maxfps / 5;
            }
            else if (_action == OLDMANWALK2)
            {   _action = OLDMANWALK1; _actiontime = maxfps / 5; }
        }
    
        // -- change direction --
        bool changedir = false;
        int mx, my;
        if (_xdir == LEFT) mx = (int)(((_x - _xsize / 2) - 6) / ts) + 1;
        else mx = (int)(((_x + _xsize / 2) + 6) / ts) + 1;
        // - random change -
        if (rand() % 200 == 0) changedir = true;
        // - wall or edge of map -
        my = (int)(_y / ts) + 1;
        if (tileType(getMap(mx, my)) == WALL || mx > mapWidth() || mx < 0)
        {   changedir = true; }
        // - big hole -
        my = (int)((_y + _ysize / 2) / ts) + 2;
        if (holeHere(mx, my)) changedir = true;
        // - change the direction -
        if (changedir) { if (_xdir == LEFT) _xdir = RIGHT; else _xdir = LEFT; }
    
        // -- possibility of brooding --
        if (rand() % 200 == 0)
        {   // set new status
            _status = OLDMANBROODING;
            _action = OLDMANBROODING;
            _actiontime = maxfps;
        }
    }
}
void Ancient::updateFrame()
{
    int frameslot, anmoffset = 0;
    if (_status == OLDMANWALKING)
    {   if (_xdir == LEFT) frameslot = 42; else frameslot = 44;
        anmoffset = _action;
    }
    else if (_status == OLDMANBROODING)
    {   if (_xdir == LEFT) frameslot = 43; else frameslot = 45; }

    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SpearUp::SpearUp(float x, float y)
:SpearEnemy(SPEARUP, x, y, 46, 12, 16) {}
void SpearUp::doAI()
{
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- handle status ---
    if (_actiontime == 0)
    {   if (_status == SPEARWAIT)
        {   _status = SPEARATTACK; _action = 0; _actiontime = maxfps / 8; }
        else if (_status == SPEARATTACK)
        {   _action++; _actiontime = maxfps / 8;
            switch (_action)
            {   case 0: break;
                case 1: _ysize = 48; break;
                case 2: _ysize = 16; break;
                case 3: _status = SPEARWAIT; _action = 0;
                        _actiontime = maxfps * 2; break;
            }
        }
    }
}
void SpearUp::updateFrame()
{
    int frameslot = 46 + _status, anmoffset = 0;
    if (_action != 2) anmoffset = _action;
    
    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SpearDown::SpearDown(float x, float y)
:SpearEnemy(SPEARDOWN, x, y, 48, 12, 16) {}
void SpearDown::doAI()
{
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- handle status ---
    if (_actiontime == 0)
    {   if (_status == SPEARWAIT)
        {   _status = SPEARATTACK; _action = 0; _actiontime = maxfps / 8; }
        else if (_status == SPEARATTACK)
        {   _action++; _actiontime = maxfps / 8;
            switch (_action)
            {   case 0: break;
                case 1: _ysize = 48; break;
                case 2: _ysize = 16; break;
                case 3: _status = SPEARWAIT; _action = 0;
                        _actiontime = maxfps * 2; break;
            }
        }
    }
}
void SpearDown::updateFrame()
{
    int frameslot = 48 + _status, anmoffset = 0;
    if (_action != 2) anmoffset = _action;
    
    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}
void SpearDown::dropToGround() { }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SpearLeft::SpearLeft(float x, float y)
:SpearEnemy(SPEARLEFT, x, y, 50, 16, 12) {}
void SpearLeft::doAI()
{
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- handle status ---
    if (_actiontime == 0)
    {   if (_status == SPEARWAIT)
        {   _status = SPEARATTACK; _action = 0; _actiontime = maxfps / 8; }
        else if (_status == SPEARATTACK)
        {   _action++; _actiontime = maxfps / 8;
            switch (_action)
            {   case 0: break;
                case 1: _xsize = 48; break;
                case 2: _xsize = 16; break;
                case 3: _status = SPEARWAIT; _action = 0;
                        _actiontime = maxfps * 2; break;
            }
        }
    }
}
void SpearLeft::updateFrame()
{
    int frameslot = 50 + _status, anmoffset = 0;
    if (_action != 2) anmoffset = _action;
    
    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}
void SpearLeft::dropToGround() { }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SpearRight::SpearRight(float x, float y)
:SpearEnemy(SPEARRIGHT, x, y, 52, 16, 12) {}
void SpearRight::doAI()
{
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- handle status ---
    if (_actiontime == 0)
    {   if (_status == SPEARWAIT)
        {   _status = SPEARATTACK; _action = 0; _actiontime = maxfps / 8; }
        else if (_status == SPEARATTACK)
        {   _action++; _actiontime = maxfps / 8;
            switch (_action)
            {   case 0: break;
                case 1: _xsize = 48; break;
                case 2: _xsize = 16; break;
                case 3: _status = SPEARWAIT; _action = 0;
                        _actiontime = maxfps * 2; break;
            }
        }
    }
}
void SpearRight::updateFrame()
{
    int frameslot = 52 + _status, anmoffset = 0;
    if (_action != 2) anmoffset = _action;
    
    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}
void SpearRight::dropToGround() { }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

ShooterUp::ShooterUp(float x, float y)
:ShooterEnemy(SHOOTERUP, x, y, 54, 16, 32) {}
void ShooterUp::doAI()
{
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- handle status
    if (_actiontime == 0)
    {   _actiontime = maxfps * 2;
        Shot::addShot(new ArrowUp(_x, _y-_ysize/2));
    }
}
void ShooterUp::updateFrame()
{
    _frame = SpriteFrame[54].tile;
    _txsize = SpriteFrame[54].width;
    _tysize = SpriteFrame[54].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

ShooterDown::ShooterDown(float x, float y)
:ShooterEnemy(SHOOTERDOWN, x, y, 55, 16, 16) {}
void ShooterDown::doAI()
{
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- handle status
    if (_actiontime == 0)
    {   _actiontime = maxfps * 2;
        Shot::addShot(new ArrowDown(_x, _y+_ysize/2));
    }
}
void ShooterDown::updateFrame()
{
    _frame = SpriteFrame[55].tile;
    _txsize = SpriteFrame[55].width;
    _tysize = SpriteFrame[55].height;
}
void ShooterDown::dropToGround() { }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

ShooterLeft::ShooterLeft(float x, float y)
:ShooterEnemy(SHOOTERLEFT, x, y, 56, 16, 16) {}
void ShooterLeft::doAI()
{
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- handle status
    if (_actiontime == 0)
    {   _actiontime = maxfps * 2;
        Shot::addShot(new ArrowLeft(_x-_xsize/2, _y));
    }
}
void ShooterLeft::updateFrame()
{
    _frame = SpriteFrame[56].tile;
    _txsize = SpriteFrame[56].width;
    _tysize = SpriteFrame[56].height;
}
void ShooterLeft::dropToGround() { }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

ShooterRight::ShooterRight(float x, float y)
:ShooterEnemy(SHOOTERRIGHT, x, y, 57, 32, 16) {}
void ShooterRight::doAI()
{
    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;
    
    // --- handle status
    if (_actiontime == 0)
    {   _actiontime = maxfps * 2;
        Shot::addShot(new ArrowRight(_x+_xsize/2, _y));
    }
}
void ShooterRight::updateFrame()
{
    _frame = SpriteFrame[57].tile;
    _txsize = SpriteFrame[57].width;
    _tysize = SpriteFrame[57].height;
}
void ShooterRight::dropToGround() { }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

MimRock::MimRock(float x, float y, int dir)
:Enemy(MIMROCK, x, y, dir, 0, 0, MIMROCKSIT, false, 58, 24, 24, 5, 6, .5) {}
int MimRock::collision(int x1, int y1, int x2, int y2)
{
    if (_status == MIMROCKSIT) return NOTANENEMY;
    int ex1 = (int)(_x - (_xsize / 2)); int ey1 = (int)(_y - (_ysize / 2));
    int ex2 = (int)(_x + (_xsize / 2)); int ey2 = (int)(_y + (_ysize / 2));

    if (ex1 >= x1 && ex1 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey1 >= y1 && ey1 <= y2) return DEADLYENEMY;
    if (ex1 >= x1 && ex1 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (ex2 >= x1 && ex2 <= x2 && ey2 >= y1 && ey2 <= y2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y1 >= ey1 && y1 <= ey2) return DEADLYENEMY;
    if (x1 >= ex1 && x1 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    if (x2 >= ex1 && x2 <= ex2 && y2 >= ey1 && y2 <= ey2) return DEADLYENEMY;
    return NOTANENEMY;
}
void MimRock::collideWithShots()
{   if (_status != ENEMYDEAD &&
        _status != MIMROCKSIT &&
        Shot::collideShot((int)(_x-_xsize/2), (int)(_y-_ysize/2),
        (int)(_x+_xsize/2), (int)(_y+_ysize/2), PLAYERSHOT)) kill();
}
void MimRock::doAI()
{
    if (_status == ENEMYDEAD) return;
    
    // --- direction to chase player ---
    int newdir;
    if (getPlayerX() > _x) newdir = RIGHT; else newdir = LEFT;

    // --- handle action timer ---
    if (_actiontime > 0) _actiontime--;

    // -- player facing this enemy --
    bool playerFacing=false;
    if ((getPlayerX() > _x && getPlayerXdir() == LEFT) ||
        (getPlayerX() < _x && getPlayerXdir() == RIGHT)) playerFacing=true;

    if (_status == MIMROCKSIT)
    {   if (_actiontime == 0 && !playerFacing && inRangeY(100))
        {   _xdir = newdir;
            if (inRange(80) && betweenRangesX(50, 80))
            {   _status = MIMROCKJUMP; _actiontime = 4; }
            else if (betweenRangesX(45, 270))
            {   _status = MIMROCKSNEAK; _actiontime = maxfps / 3; }
        }
    }
    else if (_status == MIMROCKSNEAK)
    {   _xdir = newdir;
        if (_actiontime == 0 || playerFacing)
        {   _status = MIMROCKSIT; _actiontime = maxfps; }
        else
        {   if (inRange(80) && betweenRangesX(50, 80))
            {   _status = MIMROCKJUMP; _actiontime = 4; }
            else
            {   if (_xdir == LEFT) _xv -= 1; else _xv += 1;
                if (_xv > _xmax/2) _xv = _xmax / 2;
                if (_xv < -_xmax/2) _xv = -_xmax / 2;
            }
        }
    }
    else if (_status == MIMROCKJUMP)
    {   if (!_onground || _actiontime > 0)
        {   if (_actiontime > 0) _yv -= 2;
            if (_xdir == LEFT) _xv -= 2; else _xv += 2;
        }
        else
        {   _status = MIMROCKONHEAD; _actiontime = 2; }
    }
    else if (_status == MIMROCKONHEAD)
    {   if (!_onground || _actiontime > 0)
        {   if (_actiontime > 0) _yv -= 2;
            if (_xdir == LEFT) _xv -= 2; else _xv += 2;
        }
        else if (_onground)
        {   _status = MIMROCKSIT; _actiontime = maxfps; }
    }
}
void MimRock::updateFrame()
{   /*
    58 - sitting                  62 - jump right
    59 - sneak left (4 frames)    63 - onhead left (2 frames)
    60 - sneak right (4 frames)   64 - onhead right (2 frames)
    61 - jump left                65 - dead                     */
    
    int frameslot, anmoffset = 0;
    switch (_status)
    {   case MIMROCKSIT: frameslot = 58; break;
        case MIMROCKSNEAK: anmoffset = anmCounter() % 4;
        if (_xdir == LEFT) frameslot = 59; else frameslot = 60; break;
        case MIMROCKJUMP:
        if (_xdir == LEFT) frameslot = 61; else frameslot = 62; break;
        case MIMROCKONHEAD: anmoffset = (int)(anmCounter() / 4) % 2;
        if (_xdir == LEFT) frameslot = 63; else frameslot = 64; break;
        case ENEMYDEAD: frameslot = 65;
    }
    
    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Cloud::Cloud(float x, float y, int dir)
:Enemy(CLOUD, x, y, dir, 0, 0, CLOUDSIT, false, 66, 64, 24, 1, 0, 0) {}
int Cloud::collision(int x1, int y1, int x2, int y2) { return NOTANENEMY; }
void Cloud::collideWithShots() {}
void Cloud::doAI()
{
    // --- direction to chase player ---
    int newdir;
    if (getPlayerX() > _x) newdir = RIGHT; else newdir = LEFT;
    
    // --- Handle action timer ---
    if (_actiontime > 0) _actiontime--;
    
    if (_status == CLOUDSIT)
    {   // Activate cloud by touching it
        if (playerCollision((int)(_x - _xsize / 2),(int)(_y - _ysize / 2),
            (int)(_x + _xsize / 2),(int)(_y + _ysize / 2)))
        {   _status = CLOUDROAM; _xdir = newdir;
        }
    }
    else if (_status == CLOUDROAM)
    {   // Move cloud
        if (_xdir == LEFT) _xv = -_xmax; else _xv = _xmax;
        // stay on map
        bool changedir = false;
        if (_x + 4 > mapWidth() * ts) { _xdir = LEFT; _xv = 0; }
        if (_x - 4 < 0) { _xdir = RIGHT; _xv = 0; }
        // randomly change direction
        if (rand() % 150 == 0)
        {   if (_xdir == LEFT) _xdir = RIGHT; else _xdir = LEFT;
            _xv = 0;
        }
        else if (betweenRangesX(280, 400))
        {   _xdir = newdir; _xv = 0; }
        // strike if in range
        if (inRange(100) && _y <= getPlayerY() && betweenRangesX(0, 32) &&
            _actiontime == 0)
        {   _status = CLOUDSTRIKE; _actiontime = (int)(maxfps * 1.2); }
    }
    else if (_status == CLOUDSTRIKE)
    {   if (_actiontime == (int)(maxfps * .75))
        {   int x = (int)_x;
            int y = (int)(_y + _ysize / 2 + SpriteFrame[68].height / 2);
            int sframe = SpriteFrame[68].tile; int eframe = sframe + 1;
            int xsize = SpriteFrame[68].width;
            int ysize = SpriteFrame[68].height;
            Sprite::addSprite(x, y, sframe, eframe, maxfps / 10, xsize,
                ysize, true, true, maxfps / 2);
        }
        if (_actiontime == 0)
        {   _status = CLOUDROAM; _xdir = newdir; _actiontime = maxfps / 2; }
    }
}
void Cloud::updateFrame()
{
    int frameslot, anmoffset = 0;
    if (_status == CLOUDSIT) frameslot = 66;
    if (_status == CLOUDROAM) frameslot = 67;
    if (_status == CLOUDSTRIKE)
    {   frameslot = 67; anmoffset = anmCounter() % 2; }

    _frame = SpriteFrame[frameslot].tile + anmoffset;
    _txsize = SpriteFrame[frameslot].width;
    _tysize = SpriteFrame[frameslot].height;
}
void Cloud::dropToGround() { }
