#include "Player.h"
#include "Bokobolin.h"
#include <iostream>

int main(){
    auto MainPlayer= std::make_unique<Player>();
    auto Enemy = std::make_unique<Bokobolin>();

    MainPlayer -> TargetEnemy = Enemy.get();
    std::cout << "Base X : " << MainPlayer -> basePosX << "Base Y : " << MainPlayer-> basePosY << std::endl;
    std::cout << "Base X : " << Enemy -> basePosX << " Base Y : " << Enemy -> basePosY << std::endl;
    for (int i=0;i<56;i++){
        std::cout << "Frame " << i+1 << std::endl;
        MainPlayer -> Attack();
        std::cout <<"Attack Physics = X : " << MainPlayer-> posX << " Y : " << MainPlayer -> posY << std::endl;
        if (MainPlayer->isKnocking){
            std::cout << "Enemy is knocked" << std::endl;
            Enemy -> KnockBack(MainPlayer->vX, MainPlayer-> vY);
            MainPlayer -> isKnocking = false;
            Enemy -> isKnocked = true;
        }
        if(Enemy-> isKnocked)
            Enemy -> UpdateKnock();
            
        if (MainPlayer -> posX == MainPlayer -> basePosX && MainPlayer -> posY == MainPlayer -> basePosY){ // When done attack & return
            break;
        }

    }
}