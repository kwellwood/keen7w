#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <allegro.h>
using namespace std;
#include "graphicslib.h"

/* ------------------------------------------------------------------------- *
 * GraphicsLib()                                                             *
 *                                                                           *
 * Empty constructor.                                                        *
 * ------------------------------------------------------------------------- */
GraphicsLib::GraphicsLib()
{ _error=true; }

/* ------------------------------------------------------------------------- *
 * GraphicsLib(string list)                                                  *
 *   list: filename of the tile list                                         *
 *                                                                           *
 * Loads all of the tiles from the list file.                                *
 * ------------------------------------------------------------------------- */
GraphicsLib::GraphicsLib(string list)
{
    _error=false;
    load(list);
}

/* ------------------------------------------------------------------------- *
 * ~GraphicsLib()                                                            *
 *                                                                           *
 * The destructor.                                                           *
 * ------------------------------------------------------------------------- */
GraphicsLib::~GraphicsLib()
{
    //for (unsigned int i=0; i<_tiles.size(); i++)
    //    destroy_bitmap(_tiles[i]);
    _tiles.clear();

    _tileType.clear();
    _tileType2.clear();
    _anmLength.clear();
    _anmDelay.clear();
    _slopeMask.clear();
}

/* ------------------------------------------------------------------------- *
 * void load()                                                               *
 *                                                                           *
 * Loads the files and attributes in the list file.                          *
 * ------------------------------------------------------------------------- */
void GraphicsLib::load(string list)
{
    packfile_password(string(string("billyb")+string("laze")).c_str());

    // Read the string -> int map for filename to index number in graphics.dat
    ifstream file("graphics.tbl");
    if (!file.is_open()) { setError("Can't open file GRAPHICS.TBL"); return; }
    map<string, int> fileindex;
    while (!file.eof())
    {   string filename; int indexnum;
        file >> filename; file >> indexnum;
        fileindex[filename] = indexnum;
    }
    
    // Load the entire graphics file into memory
    DATAFILE* datafile = load_datafile("GRAPHICS.DAT");
    if (datafile == NULL) { setError("Can't open file GRAPHICS.DAT"); return; }

    // Open the tiles list file
    ifstream listFile(list.c_str());
    if (!listFile.is_open())
    {   setError("Can't open file '"+list+"'"); return; }

    while (!listFile.eof())
    {
        string picfile;
        int type=0, type2=0, anmlength=0, anmdelay=0, slopemask=0;

        // Read filename, skipping blank lines and comments
        vector<int> tags;
        picfile = getNextFile(listFile, tags);

        if (picfile == "")
        {   break; }
        else if (picfile == "NO_FILE")
        {   _tiles.push_back(NULL); }           // just a placeholder
        else
        {
            // Load bitmap
            if (fileindex.find(picfile) == fileindex.end())
            {   setError("Can't open file "+picfile+" in GRAPHICS.DAT");
                return; }
            _tiles.push_back((BITMAP*)datafile[fileindex[picfile]].dat);

            // Set tileType and tileType2
            type = tags[0];
            
            if (type != 99) type2 = tags[1];
            switch (type)
            {
                // --- Slope tile ---
                case 2:
                if (tags.size() != 4)
                {   setError("Invalid tile list format"); return; }
                slopemask = tags[2];          // slope type
                anmlength = tags[3];          // overlay tag for slope tiles
                break;
                // --- Gem socket ---
                case 7:
                if (tags.size() < 4)
                {   setError("Invalid tile list format"); return; }
                slopemask = tags[2];          // key color
                anmlength = tags[3];
                if (anmlength > 0)
                {   if (tags.size() != 5)
                    {   setError("Invalid tile list format"); return; }
                    anmdelay = tags[4];
                }
                break;
                // --- tile without tags ---
                case 99:
                break;
                // --- Any other tile ---
                default:
                if (type2 == 14)              // these change into other tiles
                {   if (tags.size() != 4)
                    {   setError("Invalid tile list format"); return; }
                    slopemask = tags[2];      // tile to change to   
                    anmlength = tags[3];      // delay before change
                }
                else
                {   if (tags.size() < 3)
                    {   setError("Invalid tile list format"); return; }
                    anmlength = tags[2];
                    if (anmlength > 0)
                    {   if (tags.size() != 4)
                        {   setError("Invalid tile list format"); return; }
                        anmdelay = tags[3];
                    }
                }
                break;
            }
        }
        // Push tile attributes
        _tileType.push_back(type);
        _tileType2.push_back(type2);
        _anmLength.push_back(anmlength);
        _anmDelay.push_back(anmdelay);
        _slopeMask.push_back(slopemask);
    }
    
    fileindex.clear();
    listFile.close();
    packfile_password(NULL);
}

/* ------------------------------------------------------------------------- *
 * string getNextFile(ifstream &listFile, vector<int> &tags)                 *
 *   listFile: input file stream to read from                                *
 *   tags    : a vector to hold integer tags following the file name         *
 *                                                                           *
 * Returns true if an error occurred during tile loading.                    *
 * ------------------------------------------------------------------------- */
string GraphicsLib::getNextFile(ifstream &file, vector<int> &tags)
{
    char buffer[256];
    
    do { file.getline(buffer,256); }
    while (!file.eof() && (buffer[0] == '*' || buffer[0] == 0));
    if (file.eof()) return "";

    int cb = 0;
    string filename;
    string curtag;      // current tag text
    
    while (buffer[cb] == ' ') { cb++; }
    while (buffer[cb] != ' ' && buffer[cb] != 0)
    {   filename += buffer[cb]; cb++; }

    while (cb < 256 && buffer[cb] != 0)
    {
        if (buffer[cb] == ' ')                              // delimiting space
        {   if (curtag.length() != 0)
            {   // Add previously finished tag to vector
                const char* num = curtag.c_str();
                tags.push_back((int)strtol(num, NULL, 10));
                curtag.erase();
            }
        }
        else if (buffer[cb] >= '0' && buffer[cb] <= '9')    // a digit
        {   curtag += buffer[cb]; }
        else
        {   setError("Invalid tile list format"); return ""; }
        cb++;
    }

    // Add last tag to vector
    if (curtag.length() > 0)
    {   const char* num = curtag.c_str();
        tags.push_back((int)strtol(num, NULL, 10)); }

    return filename;
}

/* ------------------------------------------------------------------------- *
 * bool error()                                                              *
 *                                                                           *
 * Returns true if an error occurred during tile loading.                    *
 * ------------------------------------------------------------------------- */
bool GraphicsLib::error()
{   return _error; }

/* ------------------------------------------------------------------------- *
 * string errmsg()                                                           *
 *                                                                           *
 * Returns string of last error message                                      *
 * ------------------------------------------------------------------------- */
string GraphicsLib::errmsg()
{   return _errmsg; }

/* ------------------------------------------------------------------------- *
 * void setError(string e)                                                   *
 *   e: error message                                                        *
 *                                                                           *
 * Sets error to true and records error message.                             *
 * ------------------------------------------------------------------------- */
void GraphicsLib::setError(string e)
{   _error = true; _errmsg = e; }

/* ------------------------------------------------------------------------- *
 * BITMAP* get(int p)                                                        *
 *   p: The tile number                                                      *
 *                                                                           *
 * Returns the requested bitmap.                                             *
 * ------------------------------------------------------------------------- */
BITMAP* GraphicsLib::get(int p)
{
  if (p > -1 && p < _tiles.size())
    return _tiles[p];
  return NULL;
}

/* ------------------------------------------------------------------------- *
 * int tileType(int p)                                                       *
 *   p: The tile number                                                      *
 *                                                                           *
 * Returns the primary type of a tile.                                       *
 * ------------------------------------------------------------------------- */
int GraphicsLib::tileType(int p)
{
  if (p > -1 && p < _tileType.size())
    return _tileType[p];
  return 0;
}

/* ------------------------------------------------------------------------- *
 * int tileType2(int p)                                                      *
 *   p: The tile number                                                      *
 *                                                                           *
 * Returns the secondary type of a tile.                                     *
 * ------------------------------------------------------------------------- */
int GraphicsLib::tileType2(int p)
{
  if (p > -1 && p < _tileType2.size())
    return _tileType2[p];
  return 0;
}

/* ------------------------------------------------------------------------- *
 * int anmLength(int p)                                                      *
 *   p: The tile number                                                      *
 *                                                                           *
 * Returns the animation length (in frames) of a tile.                       *
 * ------------------------------------------------------------------------- */
int GraphicsLib::anmLength(int p)
{
  if (p > -1 && p < _anmLength.size())
    return _anmLength[p];
  return 0;
}

/* ------------------------------------------------------------------------- *
 * int anmDelay(int p)                                                       *
 *   p: The tile number                                                      *
 *                                                                           *
 * Returns the animation frame delay of a tile.                              *
 * ------------------------------------------------------------------------- */
int GraphicsLib::anmDelay(int p)
{
  if (p > -1 && p < _anmDelay.size())
    return _anmDelay[p];
  return 0;
}

/* ------------------------------------------------------------------------- *
 * int slopeMask(int p)                                                      *
 *   p: The tile number                                                      *
 *                                                                           *
 * Returns the slope mask of a special slope tile.                           *
 * ------------------------------------------------------------------------- */
int GraphicsLib::slopeMask(int p)
{
  if (p > -1 && p < _slopeMask.size())
    return _slopeMask[p];
  return 0;
}
