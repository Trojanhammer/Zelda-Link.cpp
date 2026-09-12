#pragma once
#include <string>

class Weapon {
    public: // variable declaration
        std::string name;
        int damage;
        int durability;
        
    public: //fx declaration
        Weapon(std::string weaponName, int weaponDamage, int weaponDurability);
        void Attack();
        void Equip();
};
