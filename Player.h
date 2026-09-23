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

    public:
        Player(); // Constructor must have same name as class name and dont have void.memang macamtu.
        void Attack() override;
};