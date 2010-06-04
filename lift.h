#include <vector>
using namespace std;

#define LIFTFRAMEDELAY  1
#define LIFTSPEED       1.6

class Lift
{
    public:
    static void init();
    static void readLifts(ifstream &file);
    static void drawLifts();
    static void tickLifts();
    static int  collideLifts(float x, float y);
    static Lift getLift(int id);
    static void clearLifts(); 

    float x();          // public accessors for a lift's location
    float y();          //

    private:
    class Locator
    {
        public:
        Locator(int x, int y, int delay, int locksw)
        :x(x), y(y), lockswitch(locksw), delay(delay)
        { }
        
        int x, y, lockswitch, delay;
    };
    
    static int Liftstoptile, Liftlefttile, Liftrighttile, Liftuptile;
    static int Liftdowntile;
    static vector<Lift> Lifts;
    static vector<Locator> Locators[16];
    
    Lift(int track, int target, int id);
    void draw();
    void tick();
    bool nearTarget();
    void setNewTarget();
    bool collide(float x, float y);
    
    float _x, _y, _xv, _yv;
    int   _delay, _target, _track, _id;
};