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
