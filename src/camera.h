class Camera
{
    public:
    Camera(Player* player);
    void tick();
    void focus();
    void move(int movx, int movy);
    bool inView(float x1, float y1, float x2, float y2);

    float x, y, vx1, vy1, vx2, vy2;

    protected:
    Player* _player;
};

class TEDCamera : public Camera
{
    public:
    TEDCamera(Player* player);
    void tick();
    void focus();
    bool inView(float x1, float y1, float x2, float y2);
    void zoomIn();
    void zoomOut();
    
    float zoom;
};
