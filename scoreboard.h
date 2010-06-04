class Scoreboard
{
    public:
    Scoreboard(Player* player);
    void tick();
    void draw();
    void drawDropdown();
    void activateDropdown();
    bool dropdownActive();

    private:
    void init();
    void drawNumber(int digits, int x, int y, int n);
    
    Player* _player;
    int  _scoreboardtile, _scoredigit, _boardx, _boardy;
    int  _dropdowntile, _dropdownheight;
    bool _dropdownactivating;
};