#pragma once
#include <string>
#include "weapon.h"
#include "Enemy.h"

class Player {
    public:
        std::string name;
        int health;
        int maxHealth;
        bool isAlive;
        float posX;
        float posY;
        int basePosX;
        int basePosY;
        float vX; 
        float vY;
        float NormalizedHypotenous;
        Weapon* EquippedWeapon; // Variable to store the pointer to the equipped weapon
        Enemy* TargetEnemy; 
    public:
        Player(std::string playerName, int playerHealth, int playerMaxHealth, bool playerIsAlive,float posX,float posY,int basePosX,
        int basePosY,float vX,float vY,float NormalizedHypotenous,Weapon* equippedWeapon, Enemy* targetEnemy);
        void TakeDamage(int damage);
        void Attack();
        
};