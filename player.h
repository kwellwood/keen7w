#define MAXPLAYERFRAMES 67

class Player
{
    public:
    Player();
    ~Player();
    void initState();
    void resetForLevel();
    void draw();
    bool collision(int x1, int y1, int x2, int y2);
    void move(int d);
    void togglePogo();
    void shoot();
    void tryFlipSwitch();
    void tryTeleport();
    void tryGrabPole();
    void climbUpPole();
    void climbDownPole();
    void climbUpLedge();
    void tryUnlockDoor();
    void tryEnterLevel();
    void kill();
    void tick();
    void readMap(ifstream &file);

    void setx(float x);
    void sety(float y);
    void setmapx(float x);
    void setmapy(float y);
    void setloc(float x, float y);
    void setxdir(int xdir);
    void setmapxdir(int xdir);
    void setxsize(int xs);
    void setysize(int ys);
    void setxmax(float xm);
    void setymax(float ym);
    void setjumpheight(float jh);
    void setinvincible(bool i);
    void setlives(int l);
    void setdead(bool d);
    void setammo(int a);
    void setscore(int s);
    void setnextlifeat(int n);
    void setdrops(int d);
    void setancientsrescued(int a);
    void setaim(int a);
    void setholdingjump(bool h);
    void setstoppedjump(bool s);
    void setkeyred();
    void setkeyyel();
    void setkeyblu();
    void setkeygrn();
    void notIdle();

    float x();
    float y();
    float mapx();
    float mapy();
    int   xdir();
    int   mapxdir();
    bool  dead();
    int   action();
    bool  invincible();
    bool  onground();
    int   onlift();
    int   onball();
    bool  holdingjump();
    int   lives();
    int   score();
    int   nextlifeat();
    int   ammo();
    int   drops();
    int   ancientsrescued();
    bool  keyred();
    bool  keyyel();
    bool  keyblu();
    bool  keygrn();
    bool  scuba();

    static float Friction;
    static float Gravity;

    private:
    void doPhysics();
    void doPogoStick();
    void doOnGround();
    void doOnLift();
    void doOnBall();
    void clipToMap();
    void hitWall(int p, float hx, float hy);
    void hitSlope(float x, float y, int l);
    void collideWithItems();
    void collideWithSprites();
    void collideWithShots();
    void collideWithEnemies();
    void checkForLedge(int side, int mx, int my);
    void finishClimbingLedge();
    void teleport();
    void updateState();
    void setFrameShooting();
    void setFrameMoving();
    void setFrameSwitching();
    void setFrameDying();
    void setFrameTeleporting();
    void setFrameClimbing();
    void setFramePogo();
    void setFrameHanging();
    void setFrameHangClimb();
    void setFrameIdle();
    void mapModeDoPhysics();
    void mapModeClipping();
    void mapModeHitWall(int p, float hx, float hy);
    void mapModeUpdateState();
    void updateClipPoints();

    float _x, _y, _mapx, _mapy, _xv, _yv, _xmax, _ymax;
    int   _xdir, _mapxdir;
    int   _sframe, _eframe, _frame, _delay;
    int   _xsize, _ysize, _txsize, _tysize;
    bool  _onground, _onslope;
    int   _onlift, _onball;
    int   _adelay;
    int   _canjump;
    float _jumpheight;
    int   _score, _nextlifeat, _lives;
    int   _ammo, _drops;
    bool  _keyred, _keyblu, _keygrn, _keyyel, _scuba;
    int   _reload, _shooting, _aim;
    int   _action, _actiontime, _changeaction, _idletime;
    bool  _frozen, _invincible;
    int   _usingportal;
    bool  _holdingjump, _stoppedjump;
    bool  _dead;
    int   _ancientsrescued;

    struct coordinate { float x; float y; } _clippoint[11];
    struct spriteframe { int tile; int width; int height; }
           _spriteFrame[MAXPLAYERFRAMES+1];
};