#pragma once
#include <string>
class Player; // tells the compiler that this class is exist and dont worry about details.trust me. this is declaration.

class Enemy{
    public:
        std::string name;
        int health;
        int maxHealth;
        int attackPower;
        bool isAlive;
        float posX;
        float posY;
        int basePosX;
        int basePosY;
        float vY; // velocityY
        float vX;
        float NormalizedHypotenous;
        static Player* MainPlayer;

    // Strucks is to group related variables together.
    // Struct to hold the stats of the enemy. struct NameOfStructType { variableType variableName; ... };
    // "EnemyStats" is not the name of struct but struct type
        struct EnemyStats{
            std::string name;
            int health;
            int maxHealth;
            int attackPower;
            bool isAlive;
            float posX;
            float posY;
            int basePosX;
            int basePosY;
            float vX; 
            float vY;
            float NormalizedHypotenous;
        };

    // Why need to declare two times.because those variables lives inside Enemy::EnemyStats not Enemy class.So should declare again
    // But once the construtor is finished, those variables will vanished

    public:
        // stats is a variable name for EnemyStats struct type.
        Enemy(EnemyStats stats);
        void TakeDamage(int damage);

        // "virtual" means this fx can have many forms for child class (polymorphism).
        virtual void TakeAction();
        void Attack();

        virtual ~Enemy() = default; // destructor
};