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
    Nosound = false;
    if (install_sound(DIGI_AUTODETECT, MIDI_NONE, 0) < 0)
    {   Nosound = true; }
    
    Music = NULL;
}

/* ------------------------------------------------------------------------- *
 * static void playMusic()                                                   *
 *                                                                           *
 * Loads a music file from the /music/ directory and begins playing it. Any  *
 * previously playing music willed be stopped.                               *
 * ------------------------------------------------------------------------- */
void Audio::playMusic(string filename)
{
    if (Nosound) return;

    ifstream file(string("./music/"+filename).c_str(), ios::in|ios::binary);
    if (!file.is_open()) error("Can't open file ./MUSIC/"+filename);
    
    // --- Get file size ---
    long start, end, filesize;
    start = file.tellg();
    file.seekg(0,ios::end); end = file.tellg();
    file.seekg(0,ios::beg); filesize = end-start;
    
    // --- Read data ---
    char data[filesize];
    file.read(&data, filesize);
    file.close();
    
    // --- Clean up old music and play new music ---
    if (Music != NULL)
    {   almp3_stop_autopoll_mp3(Music); almp3_destroy_mp3(Music); }
    Music = almp3_create_mp3(data, filesize);
    if (Music == NULL) error("Can't load music ./MUSIC/"+filename);
    almp3_play_ex_mp3(Music, 32768, 255, 127, 1000, true);
    
    almp3_start_autopoll_mp3(Music, 100);
}

/* ------------------------------------------------------------------------- *
 * static void stopMusic()                                                   *
 *                                                                           *
 * Stops the music if it is playing.                                         *
 * ------------------------------------------------------------------------- */
void Audio::stopMusic()
{
    if (Music != NULL)
    {   almp3_stop_autopoll_mp3(Music);
        almp3_destroy_mp3(Music);
        Music = NULL;
    }
}

/* ------------------------------------------------------------------------- *
 * static void stopMusic()                                                   *
 *                                                                           *
 * Frees up memory used by audio and stops any active autopolling allegro    *
 * interrupts.                                                               *
 * ------------------------------------------------------------------------- */
void Audio::shutdown()
{   if (Music != NULL)
    {   almp3_stop_autopoll_mp3(Music); almp3_destroy_mp3(Music); }
}