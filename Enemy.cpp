#include "Enemy.h"
#include "Player.h"
#include <iostream>
#include "CombatUtils.h"

Enemy::Enemy(Enemy::EnemyStats stats) {
    name = stats.name;
    health = stats.health;
    maxHealth = stats.maxHealth;
    attackPower = stats.attackPower;
    isAlive = stats.isAlive;
    posX = stats.posX;
    posY = stats.posY;
    vX = stats.vX;
    vY = stats.vY;
    basePosX = stats.basePosX;
    basePosY = stats.basePosY;
    NormalizedHypotenous = stats.NormalizedHypotenous;

}
Player* Enemy::MainPlayer = nullptr;

void Enemy::TakeDamage(int damage){  
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
    // Assume use same speed which is 8 px per frame
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
    vX = 8 * ratioX;
    vY = 8 * ratioY;

     // Check if already arrived or not to prevent wall bug
    if (NormalizedHypotenous <= 8){
        posX = MainPlayer -> posX;
        posY = MainPlayer -> posY;

    }
    else{
    posX += vX;
    posY += vY;
    }
}