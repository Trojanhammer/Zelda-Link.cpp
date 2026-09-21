#pragma once
#include <string>
#include <cstdint>
class Player; // tells the compiler that this class is exist and dont worry about details.trust me. this is declaration.

class Enemy{
    public:
        std::string name;
        int health;
        u_int8_t maxHealth;
        u_int8_t attackPower;
        bool isAlive = true;
        bool isAttack = false;
        bool isKnocking = false;
        bool isKnocked = false;
        float posX;
        float posY;
        u_int16_t basePosX;
        u_int16_t basePosY;
        float vY; // velocityY
        float vX;
        float NormalizedHypotenous =0;
        static Player* MainPlayer;

    // Strucks is to group related variables together.
    // Struct to hold the stats of the enemy. struct NameOfStructType { variableType variableName; ... };
    // "EnemyStats" is not the name of struct but struct type
        struct EnemyStats{
            std::string name;
            int health;
            uint16_t maxHealth;
            uint8_t attackPower;
            float posX;
            float posY;
            uint16_t basePosX;
            uint16_t basePosY;
            float vX; 
            float vY;
        };

    // Why need to declare two times.because those variables lives inside Enemy::EnemyStats not Enemy class.So should declare again
    // But once the construtor is finished, those variables will vanished

    public:
        // stats is a variable name for EnemyStats struct type.
        Enemy(EnemyStats stats);
        void TakeDamage(u_int8_t damage);

        // "virtual" means this fx can have many forms for child class (polymorphism).
        virtual void TakeAction();
        void Attack();
        void KnockBack(float vXOpponent, float vYOpponent);
        void UpdateKnock();

        virtual ~Enemy() = default; // destructor
};