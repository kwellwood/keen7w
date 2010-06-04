#include <vector>
using namespace std;

class Sprite
{
    public:
    static int  addSprite(float x, float y, int sframe, int eframe, int delay,
        int xsize, int ysize, bool deadly, bool repeat, int lifespan);
    static void killSprite(int s);
    static void killSpriteByID(int id);
    static Sprite* getSpriteByID(int id);
    static void tickSprites();
    static void drawSprites();
    static int  collideSprite(int x1, int y1, int x2, int y2);
    static void clearSprites();
    
    void move(float movx, float movy);
    
    private:
    Sprite(float x, float y, int sframe, int eframe, int delay, int xsize,
           int ysize, bool deadly, bool repeat, int lifespan, int id);
    void tick();
    void draw();
    bool collision(int x1, int y1, int x2, int y2);
    bool destroy();
    
    static vector<Sprite*> Sprites;
    static int NextID;
    
    float _x, _y;
    int  _frame, _sframe, _eframe, _delay, _ticks, _lifespan, _id;
    int  _xsize, _ysize;
    bool _deadly, _repeat, _destroy;
};