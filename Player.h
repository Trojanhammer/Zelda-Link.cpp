#pragma once
#include <string>
#include "weapon.h"
#include <vector>
#include <utility>

class Enemy;

class Player {
    public:
        std::string name = "Link";
        int health = 100;
        int maxHealth = 100;
        bool isAlive = true;
        bool isReturn = false;
        bool isAttack = false;
        bool isKnocking = false;
        bool isKnocked = false;
        float posX = 150;
        float posY = 300;
        int basePosX = 150;
        int basePosY = 300;
        float vX = 0;
        float vY = 0;
        float NormalizedHypotenous = 0;
        Weapon* EquippedWeapon = nullptr; // Variable to store the pointer to the equipped weapon
        Enemy* TargetEnemy = nullptr;
        std::vector<std::pair<float, float>>Recall;
    public:
        void TakeDamage(int damage);
        void Attack();
        void Return();
        void KnockBack(float VxOpponent, float VyOpponent);
        void UpdateKnock();
};