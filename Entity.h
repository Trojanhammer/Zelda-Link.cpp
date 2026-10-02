#pragma once
#include <vector>
#include <utility>
#include <string>

class Entity{
    public :
        static constexpr float SPEED =360.0f;
        std::string name;
        int health;
        u_int8_t maxHealth;
        u_int8_t attackPower;
        bool isAlive = true;
        bool isAttack = false;
        float posX;
        float posY;
        u_int16_t basePosX;
        u_int16_t basePosY;
        float vY; // velocityY
        float vX;
        float NormalizedHypotenous = 0;
        std::vector<std::pair<float,float>>RecallAttack;
        std::vector<std::pair<float,float>>RecallKnocked;

        enum GameState {
            Idle,
            DealDamage,
            Attacking,
            Knocking,
            Knocked,
            Recalling_After_Attack,
            Recalling_After_Knocked
        };
        // Fact : When we make an object of Entity class or child class for this enumerator,
        // it doesnt allocate each value inside RAM.It just stores 1 byte/4 byte just to hold currentState value
        // Those idle,dealdamage etc is just choice that object can have.
        // e;g . if it is idle, then currentstate stores value of "0" inside object
        GameState currentState = Idle;

    public:
        void TakeDamage(u_int8_t damage);
        void Attack(float posX, float posY,float deltaTime);
        void KnockBack(float vXOpponent, float vYOpponent);
        void UpdateKnock(float deltaTime);
        void Return(std::vector<std::pair <float,float>>& Recall);

        virtual ~Entity() = default; // destructor

};