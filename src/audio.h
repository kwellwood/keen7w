#include <vector>
#include <almp3.h>
using namespace std;

struct SoundData { SAMPLE* samp; };

class Audio
{
    public:
    static void init();
    static bool loadMusic(string filename);
    static void playMusic();
    static void stopMusic();
    static void tickMusic();
    static void playSound(int snd);
    static void toggleSoundfx();
    static void toggleMusic();
    static void shutdown();
    
    static bool Nosound;
    static int  MusicVol;
    static int  SoundfxVol;
    static ALMP3_MP3* Music;
    
    private:
    static DATAFILE*  MusicData;
    static vector<SoundData> SoundFX;
};
