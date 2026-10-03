#pragma once
#include <SDL.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "GameState.h"
#include "clip.h"
#include "Enemy.h"
#include "Player.h"
#include "weapon.h"

// Everything DrawScreens() needs to know about the game, bundled in one struct so main() can hand it over in one go.
// A member with & is NOT a copy: it is the same variable that main() has. When main() changes currentState,
// DrawScreens() sees the new value on the next frame. (const = DrawScreens() may look at it but not change it.)
// The order below is the order main() fills it in.
struct ScreenView {
    SDL_Renderer* renderer;
    SDL_Texture* linkTexture;
    SDL_Texture* bokobolinTexture;
    SDL_Texture* stalfosTexture;
    const float* spriteScale;                 // one number per level: 1.0, 1.25, 1.5
    const GameState& currentState;
    const uint8_t& currentLevel;              // 0 = level 1
    const uint8_t maxLevel;
    const int& currentEnemyIndex;             // whose turn it is during the enemy turn
    const Player& player;
    const std::vector<std::vector<std::unique_ptr<Enemy>>>& levelEnemies;
    const Weapon& masterSword;
    const Weapon& ocarinaSword;
    Clip* const& playingClip;                 // the hit cutscene on screen now (nullptr = none)
    const std::string& clipCaption;
    const Clip& introClip;
    const float& storyTime;                   // seconds since the intro video finished
    const float typingSpeed;                  // story letters per second
    const std::vector<std::string>& storyLines;
};

// Draws one whole frame of the game: sprites, health labels, prompts, intro, cutscenes, game over / you win.
// Call it after clearing the window and before SDL_RenderPresent.
void DrawScreens(const ScreenView& view);
