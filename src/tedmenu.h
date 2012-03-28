// --- menustate states ---
#define MENU_FILE    1
#define MENU_EDIT    2
#define MENU_CREATE  4
#define MENU_MATRIX  8
#define MENU_ITEM   16
#define MENU_ENEMY  32

// --- btns[] indexes ---
#define BTN_FILE      0
  #define BTN_NEWMAP  1
  #define BTN_SAVE    2
  #define BTN_SAVEAS 55
  #define BTN_LOAD    3
  #define BTN_QUIT    4
#define BTN_EDIT      5
  #define BTN_MUSIC   6
  #define BTN_PSTART  7
  #define BTN_SWTAGS  8
  #define BTN_TILREP  9
  #define BTN_GRDSNP 10
  #define BTN_DARKBG 56
#define BTN_CREATE   11
  #define BTN_LIFT   12
  #define BTN_LOCATR 13
  #define BTN_PORTAL 14
#define BTN_MATRIX   15
#define BTN_ITEM     16
  #define BTN_NOITEM 17
  #define BTN_SODA   18
  #define BTN_GUM    19
  #define BTN_CHOCO  20
  #define BTN_CANDY  21
  #define BTN_DONUT  22
  #define BTN_ICECRM 23
  #define BTN_FLASK  24
  #define BTN_GUN    25
  #define BTN_DROP   26
  #define BTN_REDKEY 27
  #define BTN_BLUKEY 28
  #define BTN_GRNKEY 29
  #define BTN_YELKEY 30
#define BTN_ENEMY    31
  #define BTN_NOENMY 32
  #define BTN_SLUG   33
  #define BTN_BALL   34
  #define BTN_MUSH   35
  #define BTN_LICK   36
  #define BTN_ARACH  37
  #define BTN_WORM   38
  #define BTN_SNAKE  39
  #define BTN_OLDMAN 40
  #define BTN_SPEARU 41
  #define BTN_SPEARD 42
  #define BTN_SPEARL 43
  #define BTN_SPEARR 44
  #define BTN_SHOTRU 45
  #define BTN_SHOTRD 46
  #define BTN_SHOTRL 47
  #define BTN_SHOTRR 48
  #define BTN_MROCK  49
  #define BTN_CLOUD  50
#define BTN_DELETE   51
#define BTN_COORDS   52
#define BTN_MAPBG    53
#define BTN_MAPFG    54
//      BTN_SAVEAS   55
//      BTN_DARKBG   56

#define TILE_COORDS  0
#define PIXEL_COORDS 1

class Menu
{
    public:
    Menu();
    ~Menu();
    void draw(float zoom, int vpx, int vpy);
    bool leftClick(int x, int y);
    void notMouseLB();
    bool rightClick(int x, int y);
    int  chooseTile(int curtile);
    void chooseMusic();
    void promptSwitchTag(int x, int y);
    void promptCreateLocator(int &delay, int &locksw);
    void toggleMouseCoordsType();
    
    private:
    void drawObjectBar();
    void drawButton(string text, int x1, int y1, int x2, int y2, bool on,
        bool selected = false);
    void drawButton(string text, int corners[], bool on,
        bool selected = false);
    void drawMenuOpt(string text, int corners[], bool selected = false);
    void drawMenuOpt(string text, int x1, int y1, int x2, int y2,
        bool selected = false);
    void promptNewMap();
    void promptMatrixOffsetX();
    void promptMatrixOffsetY();
    void promptLiftTrack();
    void promptTileReplace();
    string getTextInput(int x, int y, int c, int maxchars, int pixwidth,
        string input = "");
    string toStr(int n);
    int    toInt(string n);
    
    unsigned int menustate;
    string   currentMap;
    int      tileChooserRow;
    int      mouseCoordsType;

    int** btns;         // button_areas[][4];
    struct objectbar
        { int x, y, dragx, dragy, mode; string text1, text2; } objbar;
};
