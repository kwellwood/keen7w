#include <vector>
using namespace std;

#define LIFTFRAMEDELAY  1
#define LIFTSPEED       1.6
#define LOCATORWIDTH    20

class Lift
{
    public:
    static void init();
    static void addLocator(int x, int y, int track, int delay, int locksw);
    static void delLocator(int id);
    static void createLift(int x, int y, int track);
    static void delLift(int id);
    static void readLifts(ifstream &file);
    static void writeMap(ofstream &file);
    static int  atLocator(int x, int y);
    static void drawLifts();
    static void tickLifts();
    static int  collideLifts(float x, float y);
    static Lift getLift(int id);
    static void clearLifts(); 
    static void clearLocators();
    static void writeSave(ofstream &file);
    static void readSave(ifstream &file);

    float x();          // public accessors for a lift's location
    float y();          //
    void draw(float zoom, int vpx, int vpy);    // only for TEDLift class

    protected:
    static string Lift::toStr(int n);

    class Locator
    {
        public:
        Locator(int x, int y, int delay, int locksw)
        :x(x), y(y), lockswitch(locksw), delay(delay)
        { id = NextID; NextID++; }
        
        void draw(int track, int id, float zoom, int vpx, int vpy);
        bool collision(int x1, int y1);
        
        int id, x, y, lockswitch, delay;

        protected:
        static int NextID;
    };
    
    static int NextID;          // unique id for locators and lifts
    static int Liftstoptile, Liftlefttile, Liftrighttile, Liftuptile;
    static int Liftdowntile;
    static vector<Lift> Lifts;
    static vector<Locator> Locators[16];
    
    Lift(int track, int target);
    Lift() {}
    void tick();
    bool nearTarget();
    void setNewTarget();
    bool collide(float x, float y);

    float _x, _y, _xv, _yv;
    int   _delay, _target, _track, _id;
};

class TEDLift : public Lift
{
    public:
    TEDLift():Lift() { };
    static void drawLifts(float zoom, int vpx, int vpy);
    static void drawLocators(float zoom, int vpx, int vpy);
};
