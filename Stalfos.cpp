#include <iostream>
#include "Enemy.h"
#include "Stalfos.h"

//ClassName::FunctionName : Parent Constructor
Stalfos::Stalfos() : Enemy({.name = "Stalfos", .health = 80, .maxHealth = 80, .attackPower = 20, .posX = 250, .posY = 160, .basePosX = 250, .basePosY =160, .vX =0 ,.vY=0}){
}

void Stalfos::TakeAction(){
    std::cout << "Kaching Kaching";
}