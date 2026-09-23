#pragma once
#include <vector>
#include <utility>
#include <string>

class Entity{
    public :
        std::string name;
        int health;
        u_int8_t maxHealth;
        u_int8_t attackPower;
        bool ReturnAfterKnocked = false;
        bool isAlive = true;
        bool isAttack = false;
        bool isKnocking = false;
        bool isKnocked = false;
        bool isReturn = false;
        float posX;
        float posY;
        u_int16_t basePosX;
        u_int16_t basePosY;
        float vY; // velocityY
        float vX;
        float NormalizedHypotenous = 0;
        std::vector<std::pair<float,float>>RecallAttack;
        std::vector<std::pair<float,float>>RecallKnocked;

    public:
        void TakeDamage(u_int8_t damage);

        // "virtual" means this fx can have many forms for child class (polymorphism).
        virtual void Attack() = 0; //create own derived fx for child."0" here doesnt mean number

        void KnockBack(float vXOpponent, float vYOpponent);
        void UpdateKnock();
        void Return(std::vector<std::pair <float,float>>& Recall);

        virtual ~Entity() = default; // destructor

};