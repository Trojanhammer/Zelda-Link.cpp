#include "Enemy.h"
#include "Player.h"
#include <iostream>
#include <cstdint>
#include "CombatUtils.h"

Enemy::Enemy(Enemy::EnemyStats stats) {
    name = stats.name;
    health = stats.health;
    maxHealth = stats.maxHealth;
    attackPower = stats.attackPower;
    posX = stats.posX;
    posY = stats.posY;
    vX = stats.vX;
    vY = stats.vY;
    basePosX = stats.basePosX;
    basePosY = stats.basePosY;
}
Player* Enemy::MainPlayer = nullptr;

