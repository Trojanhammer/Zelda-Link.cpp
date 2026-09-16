#include <iostream>
#pragma once

inline void ApplyDamage(int& health, int& damage,bool& isAlive){
    health -=damage;
    if (health<=0){
        isAlive=false;
        std::cout << "DEAD";
    }
    else{
        std::cout << "Health " << health << " left.";
    }

}

// Use inline because in C++, it treats only one fx definition(full fx body) can only exist in one program
// without inline, Enemy.cpp and Player.cpp actually have identical fx which is ApplyDamage()
// and the linker sees two same fx definitions
// so use inline , so it tells the linker that both used identical fx definition.not seperate one
// ODD RULE (One Definition Rule)
