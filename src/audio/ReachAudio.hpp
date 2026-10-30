#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace noble {

// Halo: Reach's own menu sounds, and menu music from every Halo game in the player's MCC install
// that Noble can read (Reach, Halo: CE classic + Anniversary, Halo 3, Halo 3: ODST).
//
// Reach / Halo 3 / ODST: <game>\fmod\pc\sfx.fsb (FMOD bank) with sfx.fsb.info, 280-byte records
// {u32 sound id, u32 size, u32 2, u32 2, u64 0, char name[256]} in bank order (record N = vgmstream
// sub-song N+1). Halo: CE: halo1\sound\pc\sounds_music.fsb with lst\sounds_music.lst.bin
// {u32 1, u32 count, then per sound tag: u32 len, name (len incl. NUL), u32 first, u32 permutations}.
// The game's music is stored as pieces (in / loop / out); a "track" here is one music cue, played as
// intro, loop pieces in random order, outro.
//
// Noble decodes what it plays with vgmstream-cli into data\userdata\cache\audio (git-ignored, never
// shipped, trimmed to ~1.5 GB) and plays it with XAudio2. Nothing from MCC is copied into the repo.
// vgmstream-cli.exe is looked up in <exe>\tools\vgmstream, <exe>\vgmstream and <exe>\bin\vgmstream.
// Without it (or without MCC's Reach bank) everything is a no-op and MenuSounds falls back to tones.
namespace ReachAudio {

enum class Sfx {
    CursorVertical,    // moving up/down a list
    CursorHorizontal,  // changing a value
    Accept,            // A / Enter
    Back,              // B / Esc
    Error,
    BumperLeft,        // switching tabs
    BumperRight,
    // Post-game ("exit experience")
    GameCompletion,
    Totals,
    RankUp,
    SlotMachine,
    Count
};

void Init(const std::filesystem::path& reachDir, const std::filesystem::path& cacheDir);
void Shutdown();

// True if the sound played (it is decoded on a background thread the first time).
bool PlaySfx(Sfx sfx, float volume = 1.0f);

struct TrackInfo {
    std::string id;     // saved in settings.json (menuMusicTrack)
    std::string game;   // group heading, empty for "shuffle all" / "off"
    std::string label;
};

// Menu music. "reach:menu" = Reach's menu theme (default), any id from MusicTracks() = that track on
// repeat, "all" = every track, shuffled, "off" = silence. Old values ("shuffle", piece names) mean
// Reach's menu theme.
void PlayMenuMusic(const std::string& id);
void StopMusic();
// Pause / resume the menu music without losing its place (the XAudio2 source voice stops and starts again;
// queued pieces are kept). Returns false when no music is playing.
bool PauseMusic();
void ResumeMusic();
bool IsMusicPaused();
void SetMusicVolume(float volume);

std::vector<TrackInfo> MusicTracks();
std::string TrackLabel(const std::string& id);

// Why the audio isn't available (empty when it is).
std::string Problem();

} // namespace ReachAudio
} // namespace noble
