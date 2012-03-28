#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

// Bonus item IDs
#define SODA        1
#define GUM         2
#define CHOCOLATE   3
#define CANDY       4
#define DONUT       5
#define ICECREAM    6
#define FLASK       7
#define GUN         8
#define DROP        9
#define REDKEY     10
#define BLUKEY     11
#define GRNKEY     12
#define YELKEY     13

class Item
{
    public:
    static void addItem(Item* itm);
    static void killItem(int i);
    static void delItem(int i);
    static void readItems(ifstream &file);
    static void writeMap(ofstream &file);
    static void drawItems();
    static int  collideItem(int x1, int y1, int x2, int y2);
    static void clearItems();
    static void writeSave(ofstream &file);
    static void readSave(ifstream &file);
    static void createItem(int x, int y, int id);
    static string getName(int id);
    static int  getIcon(int id);
    Item(int x, int y, int id, int sframe, int frames, int delay,
         int xsize, int ysize);
    void draw(float zoom, int vpx, int vpy);

    protected:
    bool collision(int x1, int y1, int x2, int y2);
    virtual void giveBonus() = 0;

    static vector<Item*> Items;

    int  _x, _y, _sframe, _frames, _delay, _id, _xsize, _ysize;
};

class TEDItem : public Item
{
    public:
    TEDItem():Item(0, 0, 0, 0, 0, 0, 0, 0) { };
    static void drawItems(float zoom, int vpx, int vpy);
};

class PointsBonus : public Item
{
    public:
    PointsBonus(int x, int y, int id, int sframe, int frames, int delay,
                int xsize, int ysize, int points);
    void giveBonus();
    virtual void doPointsSprite() = 0;

    protected:
    int _points;
};

class Soda : public PointsBonus
{
    public:
    Soda(int x, int y);
    void doPointsSprite();
};

class Gum : public PointsBonus
{
    public:
    Gum(int x, int y);
    void doPointsSprite();
};

class Chocolate : public PointsBonus
{
    public:
    Chocolate(int x, int y);
    void doPointsSprite();
};

class Candy : public PointsBonus
{
    public:
    Candy(int x, int y);
    void doPointsSprite();
};

class Donut : public PointsBonus
{
    public:
    Donut(int x, int y);
    void doPointsSprite();
};

class Icecream : public PointsBonus
{
    public:
    Icecream(int x, int y);
    void doPointsSprite();
};

class Flask : public Item
{
    public:
    Flask(int x, int y);
    void giveBonus();
};

class Gun : public Item
{
    public:
    Gun(int x, int y);
    void giveBonus();
};

class Drop : public Item
{
    public:
    Drop(int x, int y);
    void giveBonus();
};

class Redkey : public Item
{
    public:
    Redkey(int x, int y);
    void giveBonus();
};

class Blukey : public Item
{
    public:
    Blukey(int x, int y);
    void giveBonus();
};

class Grnkey : public Item
{
    public:
    Grnkey(int x, int y);
    void giveBonus();
};

class Yelkey : public Item
{
    public:
    Yelkey(int x, int y);
    void giveBonus();
};
