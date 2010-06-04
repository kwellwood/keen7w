#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

class Item
{
    public:
    static void addItem(Item* itm);
    static void killItem(int i);
    static void readItems(ifstream &file);
    static void drawItems();
    static int  collideItem(int x1, int y1, int x2, int y2);
    static void clearItems();
    
    Item(int x, int y, int id, int sframe, int frames, int delay,
         int xsize, int ysize);

    protected:
    void draw();
    bool collision(int x1, int y1, int x2, int y2);
    virtual void giveBonus() = 0;

    static vector<Item*> Items;

    int  _x, _y, _sframe, _frames, _delay, _id, _xsize, _ysize;
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