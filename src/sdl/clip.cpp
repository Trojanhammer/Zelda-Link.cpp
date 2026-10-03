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
}

Clip::~Clip() {
    Free();
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

void Clip::Start() {
    started = true;
    elapsed = 0.0f;
    if (frameCount > 0) Load(0);
}

void Clip::Update(float deltaTime) {
    if (!started || frameCount == 0) return;
    elapsed += deltaTime;
    int index = std::min(frameCount - 1, (int)(elapsed * framesPerSecond));
    if (index != loadedIndex) Load(index);
}

void Clip::Skip() {
    if (!started || frameCount == 0) return;
    elapsed = frameCount / framesPerSecond;
    if (loadedIndex != frameCount - 1) Load(frameCount - 1);
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
