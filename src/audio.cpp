#include <fstream>
#include <map>
#include <allegro.h>
using namespace std;
#include "engine.h"
#include "audio.h"

/* ------------------------------------------------------------------------- *
 * static void init()                                                        *
 *                                                                           *
 * Attempts to install the sound system. If it fails, all audio will be      *
 * disabled but no error will occur.                                         *
 * ------------------------------------------------------------------------- */
void Audio::init()
{
    if (Nosound) return;
    if (install_sound(DIGI_AUTODETECT,MIDI_NONE,0) < 0)
    {   Nosound = true; return; }
    
    // Read the string -> int map for filename to index number in sounds.dat
    ifstream tblfile("sounds.tbl");
    if (!tblfile.is_open()) error("Can't open file SOUNDS.TBL");
    map<string, int> fileindex;
    while (!tblfile.eof())
    {   string filename; int indexnum;
        tblfile >> filename; tblfile >> indexnum;
        fileindex[filename] = indexnum;
    }
    tblfile.close();
    
    // --- Load the sounds datafile into memory ---
    packfile_password(string(string("bil")+string("lyblaze")).c_str());
    DATAFILE* datafile = load_datafile("SOUNDS.DAT");
    if (datafile == NULL) error("Can't open file SOUNDS.DAT");
    packfile_password(NULL);
    
    // Load sounds as listed in soundfx.dat
    ifstream file("SOUNDFX.DAT");
    if (!file.is_open()) error("Can't open file SOUNDFX.DAT");
    int numsounds;
    file >> numsounds;
    for (int i=0; i<numsounds; i++)
    {
        string filename;
        SoundData snd;
        file >> filename;

        if (fileindex.find(filename) == fileindex.end())
        {   error("Can't open file "+filename+" in SOUNDS.DAT"); }
        snd.samp = (SAMPLE*)datafile[fileindex[filename]].dat;
        SoundFX.push_back(snd);
    }
    file.close();
    
    ifstream cfgfile("AUDIO.CFG", ios::in|ios::binary);
    if (cfgfile.is_open())
    {   SoundfxVol = readInt(cfgfile);
        MusicVol = readInt(cfgfile);
        cfgfile.close();
    }
}

/* ------------------------------------------------------------------------- *
 * static bool loadMusic(string filename)                                    *
 *   filename: object name within music.dat (note: if the filename contains  *
 *             '.' in it, it will be converted to '_' to keep compatibility  *
 *             with datafile object names.                                   *
 *                                                                           *
 * Loads a music file from the music.dat archive and prepares to play it.    *
 * Use playMusic() to actually being playing music once loaded. Returns      *
 * true if music load was successful.                                        *
 * ------------------------------------------------------------------------- */
bool Audio::loadMusic(string filename)
{
    if (Nosound) return true;

    stopMusic();

    // Convert '.' in filename to _ in object names in datafile
    for (int i=0; i<filename.length(); i++)
    {   if (filename[i] == '.') filename[i] = '_'; }
    
    // --- Load data ---
    packfile_password(string(string("billybl")+string("aze")).c_str());
    MusicData = load_datafile_object("MUSIC.DAT", filename.c_str());
    packfile_password(NULL);
    if (MusicData == NULL)
    {   Music = NULL; return false; }
    
    Music = almp3_create_mp3(MusicData->dat, MusicData->size);
    //if (Music == NULL) error("Can't read music in memory");
    if (Music == NULL) { return false; }
    return true;
}

/* ------------------------------------------------------------------------- *
 * static void playMusic()                                                   *
 *                                                                           *
 * Begins playing the loaded music file. If Music == NULL, music will not    *
 * attempt to play.                                                          *
 * ------------------------------------------------------------------------- */
void Audio::playMusic()
{
    if (Nosound) return;
    
    // Play if music loaded and not playing
    if (Music != NULL)
    {   almp3_play_ex_mp3(Music, 32768, MusicVol, 127, 1000, true);
        //almp3_start_autopoll_mp3(Music, 200);
    }
}

/* ------------------------------------------------------------------------- *
 * static void stopMusic()                                                   *
 *                                                                           *
 * Stops the music if it is playing.                                         *
 * ------------------------------------------------------------------------- */
void Audio::stopMusic()
{
    if (Nosound) return;
    
    if (Music != NULL)
    {   almp3_stop_mp3(Music);
        ALMP3_MP3* temp = Music; Music = NULL;
        //almp3_stop_autopoll_mp3(Music);
        almp3_destroy_mp3(temp);
    }
    
    if (MusicData != NULL)
    {   unload_datafile_object(MusicData); MusicData = NULL; }
}

/* ------------------------------------------------------------------------- *
 * static void tickMusic()                                                   *
 *                                                                           *
 * Polls almp3 to keep the music going.                                      *
 * ------------------------------------------------------------------------- */
void Audio::tickMusic()
{   if (Music != NULL) almp3_poll_mp3(Music); }

/* ------------------------------------------------------------------------- *
 * static void playSound(int snd)                                            *
 *   snd: sound number                                                       *
 *                                                                           *
 * Begins playing the sound requested on the next sound channel.             *
 * ------------------------------------------------------------------------- */
void Audio::playSound(int snd)
{
    if (Nosound) return;
    if (snd < 1 || snd > SoundFX.size()) return;

    play_sample(SoundFX[snd-1].samp, SoundfxVol, 127, 1000, false);
}

/* ------------------------------------------------------------------------- *
 * static void toggleSoundfx()                                               *
 *                                                                           *
 * Toggles sound effects on and off.                                         *
 * ------------------------------------------------------------------------- */
void Audio::toggleSoundfx()
{
    if (Nosound) return;
    if (SoundfxVol == 0) SoundfxVol = 255; else SoundfxVol = 0;
}

/* ------------------------------------------------------------------------- *
 * static void toggleMusic()                                                 *
 *                                                                           *
 * Toggles music on and off.                                                 *
 * ------------------------------------------------------------------------- */
void Audio::toggleMusic()
{
    if (Nosound) return;
    if (MusicVol == 0) MusicVol = 127; else MusicVol = 0;
    if (Music != NULL)
    {   almp3_adjust_mp3(Music, MusicVol, 127, 1000, true); }
}

/* ------------------------------------------------------------------------- *
 * static void shutdown()                                                    *
 *                                                                           *
 * Frees up memory used by audio and stops any active autopolling allegro    *
 * interrupts.                                                               *
 * ------------------------------------------------------------------------- */
void Audio::shutdown()
{
    if (Nosound) return;

    // --- Kill music ---
    stopMusic();
    
    // --- Delete loaded sounds ---
    for (int i=0; i<SoundFX.size(); i++)
    {   destroy_sample(SoundFX[i].samp); }
    SoundFX.clear();
    
    // --- Save volume levels ---
    ofstream file("AUDIO.CFG", ios::out|ios::binary|ios::trunc);
    writeInt(file, SoundfxVol);
    writeInt(file, MusicVol);
    file.close();
}
