#include <iostream>
#include "Player.h"

Player::Player(std::string playerName, int playerHealth, int playerMaxHealth, bool playerIsAlive, Weapon* equippedWeapon, Enemy* targetEnemy)
    : name(playerName), health(playerHealth), maxHealth(playerMaxHealth), isAlive(playerIsAlive), EquippedWeapon(equippedWeapon), TargetEnemy(targetEnemy) {
}
