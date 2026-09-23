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

void Enemy::Attack(){

    if (isReturn){
        Return(RecallAttack);
        return;
    }

    // Assume use same speed which is 6 px per frame
    float dX = ((MainPlayer->posX))- posX;
    float dY = (MainPlayer ->posY) - posY;
    if(dX==0 && dY==0){ // kills this fx once arrives to location
        isReturn = true;
        Return(RecallAttack);
        isKnocking = true;
        return;
    }
    RecallAttack.push_back({posX,posY}); // record position of X and Y
    float ratioX =0;
    float ratioY = 0;
    NormalizedHypotenous = sqrt((dX * dX)+(dY * dY));
    ratioX = dX/NormalizedHypotenous;
    ratioY = dY/NormalizedHypotenous;
    vX = 6 * ratioX;
    vY = 6 * ratioY;

     // Check if already arrived or not to prevent wall bug
    if (NormalizedHypotenous <= 6){
        posX = MainPlayer -> posX;
        posY = MainPlayer -> posY;

    }
    else{
    posX += vX;
    posY += vY;
    }
}