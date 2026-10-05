#pragma once
#include <cstdint>
#include <string>

class Weapon {
    public: // variable declaration
        std::string name;
        uint8_t damage;
        uint8_t durability;
        
    public: //fx declaration
        Weapon(std::string weaponName, uint8_t weaponDamage, uint8_t weaponDurability);
        void Attack();
};
