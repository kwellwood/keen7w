#include <vector>
using namespace std;

#define MAP_VERSION  3

class Map
{
    public:
    Map(GraphicsLib* tilelib);
    ~Map();
    bool load      (string filename);
    void save      (string filename);
    void New       (int w, int h);
    void tick      ();
    void setMap    (int x, int y, int t);
    void setMapFG  (int x, int y, int t);
    int  map       (int x, int y);
    int  mapFG     (int x, int y);
    int  height    ();
    int  width     ();
    string getMusic();
    void   setMusic(string music);
    void addMorphTile (int x, int y);
    void updateMorphTiles();
    int  slopeHeight  (int x, int y, int l);
    int  hitPoint     (float x, float y);
    void setSwitchTag (int x, int y, int sw);
    int  switchTag    (int x, int y);
    void setSwitch    (int s, bool v);
    bool getSwitch    (int s);
    void toggleBridge (int loc);
    void unlockDoor   (int mx, int my);
    void replaceTile  (int t1, int t2);

    private:
    int*   _map;
    int*   _mapFG;
    int*   _switchTag;

    int    _width, _height;
    string _music;
    bool   _switch[25];

    GraphicsLib* _tilelib;
    Player*      _player;
    
    class Coordinate
    {
        public: 
        Coordinate(int x1,int y1):x(x1),y(y1){}
        int x, y;
    };
    vector<Coordinate> _morphtiles;
    int _morphtilesupdated;              // last anmcounter tick the changing
                                         // tiles were updated on
};
