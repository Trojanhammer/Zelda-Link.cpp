#include <iostream>
#include "Player.h"
#include "CombatUtils.h"
#include "Enemy.h"


void Player::TakeDamage(u_int8_t damage){
    ApplyDamage(health,damage,isAlive);

}

void Player::Attack(){

    if (isReturn){
        Return();
        return;
    }

    // Assume use same speed which is 6 px per frame
    uint8_t velocity = 6;
    float dX = ((TargetEnemy->posX)) - posX;
    float dY = (TargetEnemy ->posY) - posY;
    if(dX==0 && dY==0){
        isReturn = true;
        std::cout << "Start Return" << std::endl;
        Return ();
        isKnocking = true;
        return;
    }
    Recall.push_back({posX,posY}); // record position of X and Y
    float ratioX =0;
    float ratioY = 0;
    NormalizedHypotenous = sqrt((dX * dX)+(dY * dY));
    ratioX = dX/NormalizedHypotenous;
    ratioY = dY/NormalizedHypotenous;
    vX = velocity * ratioX;
    vY = velocity * ratioY;

    std::cout << "Hypotenous : " << NormalizedHypotenous << std::endl;

    // Check if too near or not to prevent wall bug
    if (NormalizedHypotenous <= velocity ){
        posX = TargetEnemy -> posX;
        posY = TargetEnemy -> posY;
        NormalizedHypotenous =0;
        return;
    }
    posX += vX;
    posY += vY;


}

void Player::Return(){
    // read the pair from backwards and set to posX and posY
    posX = Recall.back().first;
    posY = Recall.back().second;

    Recall.pop_back(); // Delete last pair 

    if(posX == basePosX && posY == basePosY){
        isReturn = false;
    }
}

void Player::KnockBack(float VxOpponent, float VyOpponent){
    vX = VxOpponent;
    vY = VyOpponent;
}
void Player::UpdateKnock(){
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
