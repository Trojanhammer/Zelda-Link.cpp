#pragma once
#include <string>
#include "weapon.h"
#include <vector>
#include <utility>
#include <cstdint>
#include "Entity.h"

class Enemy;

class Player : public Entity{
    public:
        Weapon* EquippedWeapon = nullptr; // Variable to store the pointer to the equipped weapon
        Enemy* TargetEnemy = nullptr;
        std::vector<std::unique_ptr<Weapon>> WeaponList; // Vector to store the player's weapons

    public:
        Player(); // Constructor must have same name as class name and dont have void.memang macamtu.
        void Next_Level_Weapon(uint8_t currentLevel);
    };