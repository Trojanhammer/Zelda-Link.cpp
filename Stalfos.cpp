#include <iostream>
#include "Enemy.h"
#include "Stalfos.h"

//ClassName::FunctionName : Parent Constructor
Stalfos::Stalfos() : Enemy({.name = "Stalfos", .health = 80, .maxHealth = 80, .attackPower = 20, .isAlive=true,.movspeed = 1}){
}

void Stalfos::TakeAction(){
    std::cout << "Kaching Kaching";
}