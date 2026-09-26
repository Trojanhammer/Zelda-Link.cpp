#include "weapon.h"
#include <iostream> 

//ClassName::FunctionName is the syntax to define fx more .Constructor
Weapon::Weapon(std::string weaponName,u_int8_t weaponDamage, u_int8_t weaponDurability) : name(weaponName), damage(weaponDamage), durability(weaponDurability) {
}

void Weapon::Attack() {
    if (durability>0){
        std::cout << "Attack with " << name << " for " << (int)damage << " damage! "<< std::endl;
        durability--;
        std::cout << "Durability left: " << (int)durability << std::endl;
    }
    else{
        std::cout << "The weapon " << name << " is broken and cannot be used " << std::endl;
    }
}
