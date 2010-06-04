#include <vector>
using namespace std;

class Shot
{
    public:
    static void addShot(Shot* shot);
    static void killShot(int s);
    static void drawShots();
    static void tickShots();
    static bool collideShot(int x1, int y1, int x2, int y2, int owner);
    static void clearShots();
    
    Shot(float x, float y, int dir, int sframe, int eframe, int delay,
        int xsize, int ysize, float velocity, int owner);
    
    protected:
    void draw();
    void tick();
    void clipToMap();
    bool collision(int x1, int y1, int x2, int y2);
    virtual void explode();
    bool inRange(int dist);

    static vector<Shot*> Shots;
    static int NextID;
    
    float _x, _y, _xv, _yv;
    int   _frame, _sframe, _eframe, _delay, _xsize, _ysize, _owner;
    bool  _destroy;
};

class Blaster : public Shot
{
    public:
    Blaster(float x, float y, int dir);
    void explode();
    
    private:
    int _expsframe, _expeframe, _expdelay, _expxsize, _expysize;
};

class ArrowUp : public Shot
{
    public:
    ArrowUp(float x, float y);
};

class ArrowDown : public Shot
{
    public:
    ArrowDown(float x, float y);
};

class ArrowLeft : public Shot
{
    public:
    ArrowLeft(float x, float y);
};

class ArrowRight : public Shot
{
    public:
    ArrowRight(float x, float y);
};