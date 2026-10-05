#pragma once
#include <vector>
#include <utility>
#include <string>
#include <cstdint>

class Entity{
    public :
        static constexpr float SPEED =360.0f;
        std::string name;
        int health;
        uint8_t maxHealth;
        uint8_t attackPower;
        bool isAlive = true;
        float posX;
        float posY;
        float RecallClock = -1.0f; // to prevent the first time Return() function to run
        uint16_t basePosX;
        uint16_t basePosY;
        float vY; // velocityY
        float vX;
        float NormalizedHypotenous = 0;
        struct RecallPoint{
            float posX;
            float posY;
            float time;
        };
        std::vector<RecallPoint> Recall_After_Attack;
        std::vector<RecallPoint> Recall_After_Knocked;
        float time = 0;
        enum EntityState {
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
        EntityState currentState = Idle;

    public:
        void TakeDamage(uint8_t damage);
        void Attack(float posX, float posY,float deltaTime);
        void KnockBack(float vXOpponent, float vYOpponent);
        void UpdateKnock(float deltaTime);
        void Return(std::vector<RecallPoint>& Recall, float deltaTime);

        virtual ~Entity() = default; // destructor

};