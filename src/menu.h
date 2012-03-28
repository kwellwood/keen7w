#include <string>

#define MAIN      1
#define OPTIONS   2
#define SAVE      3
#define LOAD      4

#define MAINITEMS     5
#define SAVELOADITEMS 8
#define OPTIONSITEMS  4
#define MENU_VERSION  2

class Menu
{
    public:
    Menu();
    void draw();
    void handleKey(int k);
    void drawLoadingBox(int level);
    void enterCheat();
    bool tryAgainMenu();
    void gotScubaGear();
    void cantSwim();
    void helpMenu();
    void ancientRescued();
    void winGame();

    private:
    void   newGame();
    void   saveGame(int slot);
    void   loadGame(int slot);
    string getSaveSlotTitle(int slot);
    void   winGameScreen(int t, int x, int y, float delay);
    void   drawTextBlock(string text, int x, int y, int width, int col);
    string getTextInput(int x, int y, int c, int maxchars, int pixwidth);
    void   init();
    
    int  _menutile, _menux, _menuy;
    int  _selectorstartfrm, _selectorframes, _selectordelay, _selectionpixels;
    int  _titlescreentile, _helpmenutile, _helpmenucursor;
    int  _instructionstile, _storytile, _tryagaintile;
    int  _loadingtile, _loadx, _loady;
    int  _ancientspeaktile, _ancspeakx, _ancspeaky;
    int  _keenspeaktile, _keenspeakx, _keenspeaky;    
    int  _menu, _menuopt;
    long _timermark;
    string _savedGameTitle[8];
};
