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

void Enemy::TakeDamage(uint8_t damage){  
    if(!isAlive){
        std::cout << name << " is already dead" << std::endl;
        return; //return is for exit the function
    };

    ApplyDamage(health,damage,isAlive);
}
void Enemy :: TakeAction(){
    std::cout << "Bam";
    
}

void Enemy::Attack(){
    // Assume use same speed which is 6 px per frame
    float dX = ((MainPlayer->posX))- posX;
    float dY = (MainPlayer ->posY) - posY;
    if(dX==0 && dY==0){ // kills this fx once arrives to location
        return;
    }
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

void Enemy::KnockBack( float vXOpponent ,float vYOpponent){
    vX = vXOpponent ;
    vY = vYOpponent;
}
void Enemy::UpdateKnock(){
    vX *= 0.90;
    vY *= 0.90;
    if(std::abs(vX) <0.1 && std::abs(vY) <0.1){
        vX=0;
        vY=0;
        isKnocked = false;
        return;
    }
    posX += vX;
    posY += vY;
    std::cout << "velo X : " << vX << " velo Y : " << vY << std::endl;
    std::cout << "X : " << posX << " Y : " << posY << "\n" <<std::endl;
}