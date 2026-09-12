#include "Enemy.h"
#include <iostream>

Enemy::Enemy(Enemy::EnemyStats stats) {
    name = stats.name;
    health = stats.health;
    maxHealth = stats.maxHealth;
    attackPower = stats.attackPower;
    isAlive = stats.isAlive;
    movspeed = stats.movspeed;
}

void Enemy::TakeDamage(int damage){  
    if(!isAlive){
        std::cout << name << " is already dead" << std::endl;
        return;
    };

    health -= damage;
    if (health<=0){
        std::cout << name << " has been defeated!" << std::endl;
        isAlive = false;
    }
    else{
        std::cout << name << " has " << health << " health left!" << std::endl;
    }
}
void Enemy :: TakeAction(){
    std::cout << "Bam";
}