#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#define PORTALWIDTH  24
#define PORTALHEIGHT 24

class Portal
{
    public:
    static void   addPortal(int x, int y, int destx = -100, int desty = -100);
    static void   addDest(int destx, int desty);
    static void   deleteLast();
    static void   delPortal(int p);
    static void   readPortals(ifstream &file);
    static void   drawPortals(float zoom, int vpx, int vpy);
    static void   writeMap(ofstream &file);
    static int    atPortal(float x, float y);
    static Portal getPortal(int p);
    static void   clearPortals();

    int  x();
    int  y();
    int  destx();
    int  desty();
    
    private:
    Portal(int x, int y, int destx, int desty);
    void draw(int id, float zoom, int vpx, int vpy);
    bool collision(float x, float y);

    static string toStr(int n);

    static int NextID;
    static vector<Portal> Portals;

    int _id, _x, _y, _destx, _desty;
};

