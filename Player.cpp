#include <iostream>
#include "Player.h"
#include "CombatUtils.h"

Player::Player(std::string playerName, int playerHealth, int playerMaxHealth, bool playerIsAlive, float positionX, float positionY, int basePositionX, int basePositionY,
    float VelocityX,float VelocityY,float NormalizedHypotenous,Weapon* equippedWeapon,Enemy* targetEnemy)
    : name(playerName), health(playerHealth), maxHealth(playerMaxHealth), isAlive(playerIsAlive), 
    posX(positionX),posY(positionY), basePosX(basePositionX), basePosY(basePositionY),vX(VelocityX),vY(VelocityY),NormalizedHypotenous(NormalizedHypotenous),
    EquippedWeapon(equippedWeapon) ,TargetEnemy(targetEnemy)
    {
    }

void Player::TakeDamage(int damage){
    ApplyDamage(health,damage,isAlive);

}

void Player::Attack(){
    // Assume use same speed which is 10 px per frame
    float dX = ((TargetEnemy->posX))- posX;
    float dY = (TargetEnemy ->posY) - posY;
    float ratioX =0;
    float ratioY = 0;
    NormalizedHypotenous = sqrt((dX * dX)+(dY * dY));
    ratioX = dX/NormalizedHypotenous;
    ratioY = dY/NormalizedHypotenous;
    vX = 10 * ratioX;
    vY = 10 * ratioY;

     // Check if already arrived or not to prevent wall bug
    if (NormalizedHypotenous <= 10){
        posX = TargetEnemy -> posX;
        posY = TargetEnemy -> posY;

    }
    else{
    posX += vX;
    posY += vY;
    }



}