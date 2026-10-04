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
void Player::Next_Level_Weapon(uint8_t currentLevel){
    uint8_t x,y;
        for (int i = 0; i < WeaponList.size(); ++i) {
            if (WeaponList[i]->name == "Master Sword") {
                x = i;
            } else if (WeaponList[i]->name == "Ocarina Sword") {
                y = i;
            }
        }
    if(currentLevel == 1){
        WeaponList[x] -> durability = 2;
        WeaponList[y] -> durability = 2;
        WeaponList[x] -> damage = 70;
        WeaponList[y] -> damage = 50;
    }
    else{
        WeaponList[x] -> durability = 2;
        WeaponList[y] -> durability = 2;
        WeaponList[x] -> damage = 100;
        WeaponList[y] -> damage = 80;
    }
 }