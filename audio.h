#include <almp3.h>
using namespace std;

class Audio
{
    public:
    static void init();
    static void playMusic(string filename);
    static void stopMusic();
    static void shutdown();
    
    private:
    static bool Nosound;
    static ALMP3_MP3* Music;
};