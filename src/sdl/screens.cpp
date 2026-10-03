#include "screens.h"
#include "ui.h"

#include <cmath>
#include <string>

namespace {
    // "[1] MASTER SWORD (2 LEFT)" or "[2] OCARINA SWORD (BROKEN)"
    std::string WeaponLabel(int key, const Weapon& weapon) {
        return "[" + std::to_string(key) + "] " + weapon.name + (weapon.durability > 0 ? " (" + std::to_string((int)weapon.durability) + " LEFT)" : " (BROKEN)");
    }

    // "[1] ATTACK BOKOBOLIN" or "BOKOBOLIN DEFEATED"
    std::string TargetLabel(int key, const Enemy& enemy) {
        return enemy.isAlive ? "[" + std::to_string(key) + "] ATTACK " + enemy.name : enemy.name + " DEFEATED";
    }
}

void DrawScreens(const ScreenView& view) {
    SDL_Renderer* renderer = view.renderer;
    const Player& player = view.player;
    const auto& enemies = view.levelEnemies[view.currentLevel];   // the enemies of this level
    Enemy* turnEnemy = enemies[view.currentEnemyIndex].get();
    bool allIdle = (player.currentState == Player::Idle) && (turnEnemy -> currentState == Enemy::Idle);

    if(view.currentState != Intro){ // the intro has the whole window to itself
        // SDL2 lukis X coord and Y lain.tapi physics in game tetap attack ke posX,posY asal.
        UI::DrawSprite(renderer, view.linkTexture, player.posX, player.posY, view.spriteScale[view.currentLevel]);
        UI::DrawSprite(renderer, view.bokobolinTexture, enemies[0] -> posX, enemies[0] -> posY, view.spriteScale[view.currentLevel]);
        UI::DrawSprite(renderer, view.stalfosTexture, enemies[1] -> posX, enemies[1] -> posY, view.spriteScale[view.currentLevel]);

        // name + health above each enemy. Uses the base position, so the label does not fly away when the enemy is knocked back
        int spriteHalfHeight = (int)(UI::TextureHeight(view.bokobolinTexture) * view.spriteScale[view.currentLevel] / 2);
        for(const auto& e : enemies){
            UI::DrawHealthLabel(renderer, e -> name, e -> health, e -> maxHealth, e -> basePosX, e -> basePosY - spriteHalfHeight - 2, e -> isAlive);
        }
        UI::DrawHealthLabel(renderer, player.name, player.health, player.maxHealth, 720, 36, player.isAlive);
        UI::DrawTextCentered(renderer, "LEVEL " + std::to_string(view.currentLevel + 1) + " / " + std::to_string((int)view.maxLevel), 400, 10, 2, UI::DARK);
    }

    if(view.playingClip != nullptr){
        view.playingClip -> Draw(renderer); // covers the whole scene
        UI::DrawPromptBar(renderer, view.clipCaption, "PRESS ANY KEY TO SKIP");
    }
    else if(view.currentState == Intro){
        view.introClip.Draw(renderer);
        if(view.introClip.FrameCount() == 0){ // frames not extracted yet
            UI::DrawTextCentered(renderer, "ZELDA-LINK", 400, 120, 8, UI::DARK, true);
        }
        if(view.introClip.Finished()){
            int typed = (int)(view.storyTime * view.typingSpeed); // 1.5 s * 40 = 60 letters so far
            UI::DrawLinesCentered(renderer, view.storyLines, 400, 372, 2, UI::DARK, typed);
            if(typed >= UI::CountLetters(view.storyLines) && std::fmod(view.storyTime, 1.0f) < 0.6f){ // blinks
                UI::DrawTextCentered(renderer, "PRESS ENTER TO START", 400, 466, 2, UI::RED);
            }
        }
    }
    else if(view.currentState == PlayerDie && allIdle){
        // Link can lose in two ways: his health runs out, or both swords are broken (he is still alive then)
        UI::DrawOverlay(renderer, "GAME OVER", player.isAlive ? "YOUR SWORDS ARE BROKEN" : "LINK HAS FALLEN", "CLOSE THE WINDOW TO QUIT");
    }
    else if(view.currentState == CompleteGame){
        UI::DrawOverlay(renderer, "YOU WIN!", "ALL " + std::to_string((int)view.maxLevel) + " LEVELS CLEARED", "CLOSE THE WINDOW TO QUIT");
    }
    else if(player.TargetEnemy != nullptr && player.EquippedWeapon != nullptr){ // Link's turn is playing out
        UI::DrawPromptBar(renderer, "LINK ATTACKS " + player.TargetEnemy -> name + "!",
                          player.EquippedWeapon -> name + ": " + std::to_string((int)player.EquippedWeapon -> damage) + " DAMAGE");
    }
    else if(view.currentState == EnemyTurn || view.currentState == PlayerDie){ // an enemy is attacking
        UI::DrawPromptBar(renderer, turnEnemy -> name + " ATTACKS LINK!", std::to_string((int)turnEnemy -> attackPower) + " DAMAGE");
    }
    else if(view.currentState == ChooseWeapon){
        // (with both swords broken main() switches to PlayerDie, so this menu always has a sword to offer)
        UI::DrawPromptBar(renderer, "CHOOSE YOUR WEAPON", WeaponLabel(1, view.masterSword) + "    " + WeaponLabel(2, view.ocarinaSword));
    }
    else if(view.currentState == ChooseEnemy){
        UI::DrawPromptBar(renderer, "CHOOSE YOUR TARGET", TargetLabel(1, *enemies[0]) + "    " + TargetLabel(2, *enemies[1]));
    }
}
