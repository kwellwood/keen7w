#include <fstream>
#include <vector>

class GraphicsLib
{
    public:
    GraphicsLib();
    GraphicsLib(string list);
    ~GraphicsLib();

    void load(string list);
    BITMAP* get(int p);
    int tileType(int p);
    int tileType2(int p);
    int anmLength(int p);
    int anmDelay(int p);
    int slopeMask(int p);
    string tileDesc(int t);
    string tileDesc2(int t);
    int size();
    bool error();
    string errmsg();

    private:
    void   setError(string e);
    string getNextFile(ifstream &file, vector<int> &tags);

    vector<BITMAP*> _tiles;
    vector<int>     _tileType,
                    _tileType2,
                    _anmLength,
                    _anmDelay,
                    _slopeMask;

    int      _numtiles;       // number of tiles in this library
    bool     _error;          // an error loading a tile
    string   _errmsg;
};
