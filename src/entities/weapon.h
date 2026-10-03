#pragma once
#include <cstdint>
#include <string>

class Weapon {
    public: // variable declaration
        std::string name;
        u_int8_t damage;
        u_int8_t durability;
        
    public: //fx declaration
        Weapon(std::string weaponName, u_int8_t weaponDamage, u_int8_t weaponDurability);
        void Attack();
};
