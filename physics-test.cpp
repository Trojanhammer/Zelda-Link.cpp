#include "Player.h"
#include "Enemy.h"
#include <iostream>

int main(){
    auto MainPlayer= std::make_unique<Player>();
    auto enemy = std::make_unique<Enemy>(Enemy::EnemyStats{.name = "Bokobolin", .health = 80, .maxHealth = 80, .attackPower = 10, .posX = 100, .posY = 50, .basePosX = 100, .basePosY = 50, .vX = 0, .vY = 0});

    MainPlayer -> TargetEnemy = enemy.get();
    std::cout << "Base X : " << MainPlayer -> basePosX << "Base Y : " << MainPlayer-> basePosY << std::endl;
    std::cout << "Base X : " << enemy -> basePosX << " Base Y : " << enemy -> basePosY << std::endl;
    for (int i=0;i<56;i++){
        std::cout << "Frame " << i+1 << std::endl;
        MainPlayer -> Attack();
        std::cout <<"Attack Physics = X : " << MainPlayer-> posX << " Y : " << MainPlayer -> posY << std::endl;
        if (MainPlayer->isKnocking){
            std::cout << "Enemy is knocked" << std::endl;
            enemy -> KnockBack(MainPlayer->vX, MainPlayer-> vY);
            MainPlayer -> isKnocking = false;
            enemy -> isKnocked = true;
        }
        if(enemy-> isKnocked)
            enemy -> UpdateKnock();
            
        if (MainPlayer -> posX == MainPlayer -> basePosX && MainPlayer -> posY == MainPlayer -> basePosY){ // When done attack & return
            break;
        }

    }
}