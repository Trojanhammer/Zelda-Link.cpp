#include "clip.h"

#include <SDL_image.h>

#include <algorithm>
#include <cstdio>

namespace {
    std::string FramePath(const std::string& folder, int index) {
        char name[16];
        std::snprintf(name, sizeof(name), "%03d.jpg", index + 1);
        return folder + "/" + name;
    }

    bool FileExists(const std::string& path) {
        FILE* file = std::fopen(path.c_str(), "rb");
        if (file == nullptr) return false;
        std::fclose(file);
        return true;
    }
}

Clip::Clip(SDL_Renderer* renderer, const std::string& folder, float framesPerSecond)
    : renderer(renderer), folder(folder), framesPerSecond(framesPerSecond) {
    while (FileExists(FramePath(folder, frameCount))) frameCount++;   // count the pictures: 001.jpg, 002.jpg, ... until one is missing

    int frequency = 0, channels = 0;
    Uint16 format = 0;
    std::string soundPath = folder + "/audio.ogg";
    if (Mix_QuerySpec(&frequency, &format, &channels) != 0 && FileExists(soundPath)) {   // the mixer must be open (main() opens it before the clips)
        sound = Mix_LoadWAV(soundPath.c_str());   // despite the name this loads .ogg too
    }
}

Clip::~Clip() {
    Free();
    int frequency = 0, channels = 0;
    Uint16 format = 0;
    if (sound != nullptr && Mix_QuerySpec(&frequency, &format, &channels) != 0) {   // only while the mixer is still open
        Mix_FreeChunk(sound);
    }
}

void Clip::Free() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    loadedIndex = -1;
}

void Clip::Load(int index) {
    Free();
    texture = IMG_LoadTexture(renderer, FramePath(folder, index).c_str());
    loadedIndex = index;
}

void Clip::StartSound() {
    if (sound == nullptr) return;
    pausedMusic = Mix_PlayingMusic() && !Mix_PausedMusic();
    if (pausedMusic) Mix_PauseMusic();
    soundChannel = Mix_PlayChannel(-1, sound, 0);   // -1 = any free channel, 0 = play once
}

void Clip::StopSound() {
    if (soundChannel >= 0 && Mix_Playing(soundChannel) && Mix_GetChunk(soundChannel) == sound) {
        Mix_HaltChannel(soundChannel);   // the clip was skipped, or its last picture is up and the sound is a few ms longer
    }
    soundChannel = -1;
    if (pausedMusic) {
        Mix_ResumeMusic();
        pausedMusic = false;
    }
}

void Clip::Start() {
    StopSound();   // in case it was still playing
    started = true;
    elapsed = 0.0f;
    if (frameCount > 0) {
        Load(0);
        StartSound();
    }
}

void Clip::Update(float deltaTime) {
    if (!started || frameCount == 0) return;
    elapsed += deltaTime;
    int index = std::min(frameCount - 1, (int)(elapsed * framesPerSecond));
    if (index != loadedIndex) Load(index);
    if (Finished()) StopSound();   // cheap and harmless when it is already stopped
}

void Clip::Skip() {
    if (!started || frameCount == 0) return;
    elapsed = frameCount / framesPerSecond;
    if (loadedIndex != frameCount - 1) Load(frameCount - 1);
    StopSound();
}

bool Clip::Finished() const {
    if (!started || frameCount == 0) return true;
    return elapsed >= frameCount / framesPerSecond;
}

void Clip::Draw(SDL_Renderer* renderer) const {
    if (!started || texture == nullptr) return;

    int windowW = 0, windowH = 0;
    SDL_GetRendererOutputSize(renderer, &windowW, &windowH);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect whole = {0, 0, windowW, windowH};
    SDL_RenderFillRect(renderer, &whole);   // hides the game underneath

    int w = 0, h = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    // e.g. a 640 x 360 picture in an 800 x 500 window: min(800/640, 500/360) = min(1.25, 1.39) = 1.25 -> 800 x 450
    float scale = std::min((float)windowW / w, (float)windowH / h);
    SDL_Rect dst;
    dst.w = (int)(w * scale);
    dst.h = (int)(h * scale);
    dst.x = (windowW - dst.w) / 2;
    dst.y = (windowH - dst.h) / 2;
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
}
