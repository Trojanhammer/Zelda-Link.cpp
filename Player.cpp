#include <iostream>
#include "Player.h"
#include "CombatUtils.h"
#include "Enemy.h"

Player::Player(){
    name = "Link";
    health = 100;
    maxHealth = 100;
    posX = 350;
    posY = 350;
    basePosX = 350;
    basePosY = 350;
}
void Player::Attack(){

    if (isReturn){
        Return(RecallAttack);
        return;
    }

    // Assume use same speed which is 6 px per frame
    uint8_t velocity = 6;
    float dX = ((TargetEnemy->posX)) - posX;
    float dY = (TargetEnemy ->posY) - posY;
    if(dX==0 && dY==0){
        isReturn = true;
        Return (RecallAttack);
        isKnocking = true;
        return;
    }
    RecallAttack.push_back({posX,posY}); // record position of X and Y
    float ratioX =0;
    float ratioY = 0;
    NormalizedHypotenous = sqrt((dX * dX)+(dY * dY));
    ratioX = dX/NormalizedHypotenous;
    ratioY = dY/NormalizedHypotenous;
    vX = velocity * ratioX;
    vY = velocity * ratioY;

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

