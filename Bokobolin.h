// Bokobolin and Stalfos is the same stats across all given same species.So just initialize(give value) in .cpp rather than in main to minimize manually typing
#include <string>
#include "Enemy.h"
#pragma once // Use #pragma once to ensure the header is only included once during compilation

class Bokobolin : public Enemy {
    public:
        //int stolenGold;
    public:
        // Cant use  "using Enemy::Enemy("Bokobolin",100,100);" because using dont support giving args
        // Should use zero args for constructor
        Bokobolin();
        // must use "override" to indicate overriding this fx from parent.otherwise it is treated as new fx
        void TakeAction() override;
};

