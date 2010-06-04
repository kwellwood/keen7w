#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#define PORTALWIDTH  24
#define PORTALHEIGHT 24

class Portal
{
    public:
    static void   addPortal(int x, int y, int destx, int desty);
    static void   readPortals(ifstream &file);
    static int    atPortal(float x, float y);
    static Portal getPortal(int p);
    static void   clearPortals();

    int  x();
    int  y();
    int  destx();
    int  desty();
    
    private:
    Portal(int x, int y, int destx, int desty);
    bool collision(float x, float y);

    static vector<Portal> Portals;

    int _x, _y, _destx, _desty;
};