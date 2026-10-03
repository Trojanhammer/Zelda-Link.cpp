#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <string>

// A short animation that is stored as numbered pictures (001.jpg, 002.jpg, ...), made from the .mp4 files by
// tools/extract_frames.sh. SDL2 cannot play .mp4, so instead of playing a video we pick the picture that matches
// how many seconds have passed:
//     12 pictures per second, 0.50 seconds in  ->  picture number 0.50 * 12 = 6 (the 7th one)
// Because it goes by seconds (deltaTime) and not by frames, a clip takes the same time at 30 fps and at 144 fps.
// If the folder has no pictures the clip is simply empty: Finished() is true at once and the game carries on.
// If the folder also has audio.ogg (the sound of the .mp4), it plays together with the pictures. The background music is
// paused while it plays and carries on from the same spot afterwards, so the two never talk over each other.
class Clip {
public:
    Clip(SDL_Renderer* renderer, const std::string& folder, float framesPerSecond);
    ~Clip();
    Clip(const Clip&) = delete;              // owns a texture, so copying it would free the texture twice
    Clip& operator=(const Clip&) = delete;

    void Start();                  // begin again from the first picture
    void Update(float deltaTime);  // call once per frame while the clip is playing
    void Skip();                   // jump to the end (the last picture stays on screen)
    bool Finished() const;         // never started, or every picture has been shown for its full time

    // Fills the whole window with white, then draws the current picture in the middle, as large as fits.
    // After the clip has finished it keeps showing the last picture (the title card at the end of the intro).
    void Draw(SDL_Renderer* renderer) const;

    void Free();                   // call before SDL_DestroyRenderer; the destructor only cleans up after this
    int FrameCount() const { return frameCount; }

private:
    void Load(int index);          // index 0 = 001.jpg
    void StartSound();             // pause the music (if it is playing) and play audio.ogg
    void StopSound();              // stop audio.ogg and let the music carry on; does nothing if it is not playing

    SDL_Renderer* renderer;
    std::string folder;
    float framesPerSecond;
    int frameCount = 0;
    int loadedIndex = -1;
    float elapsed = 0.0f;          // seconds since Start()
    bool started = false;
    SDL_Texture* texture = nullptr;   // only the current picture is kept in memory
    Mix_Chunk* sound = nullptr;       // audio.ogg, nullptr when there is none (or the mixer is not open)
    int soundChannel = -1;            // the mixer channel it plays on while it plays
    bool pausedMusic = false;         // true while we are the reason the music is paused
};
