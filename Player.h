#pragma once
#include <string>
#include "weapon.h"
#include <vector>
#include <utility>
#include <cstdint>

class Enemy;

class Player {
    public:
        std::string name = "Link";
        int health = 100;
        uint8_t maxHealth = 100;
        bool isAlive = true;
        bool isReturn = false;
        bool isAttack = false;
        bool isKnocking = false;
        bool isKnocked = false;
        float posX = 150;
        float posY = 300;
        u_int16_t basePosX = 150;
        u_int16_t basePosY = 300;
        float vX = 0;
        float vY = 0;
        float NormalizedHypotenous = 0;
        Weapon* EquippedWeapon = nullptr; // Variable to store the pointer to the equipped weapon
        Enemy* TargetEnemy = nullptr;
        std::vector<std::pair<float, float>>Recall;
    public:
        void TakeDamage(u_int8_t damage);
        void Attack();
        void Return();
        void KnockBack(float VxOpponent, float VyOpponent);
        void UpdateKnock();
};