#include "Entity.h"
#include "CombatUtils.h"

    void Entity::Return(std::vector<std::pair<float, float>>& Recall){
        // read the pair from backwards and set to posX and posY
        posX = Recall.back().first;
        posY = Recall.back().second;

        Recall.pop_back(); // Delete last pair 

        if(posX == basePosX && posY == basePosY){
            currentState = Idle;
        }
    }

    void Entity::KnockBack(float VxOpponent, float VyOpponent){
        vX = VxOpponent;
        vY = VyOpponent;
    }
    void Entity::UpdateKnock(float deltaTime){
        vX *= std::pow(0.90, deltaTime * 60.0f); // multiply by 60 to make it frame rate independent
        vY *= std::pow(0.90, deltaTime * 60.0f); 
        // alternative way to make it frame rate independent is to use std::pow(0.90, deltaTime * 60) instead of 0.90
        if(std::abs(vX*deltaTime) <0.1 && std::abs(vY*deltaTime) <0.1){
            vX=0;
            vY=0;
            currentState = Recalling;
            return;
        }
        RecallKnocked.push_back({posX, posY});
        posX += vX * deltaTime;
        posY += vY * deltaTime;
    }

    void Entity::TakeDamage(uint8_t damage){  
        if(!isAlive){
            return; //return is for exit the function
        };

        ApplyDamage(health,damage,isAlive);
    }

    void Entity::Attack(float OppositionX, float OppositionY,float deltaTime){
        // set capped at 3000px/s
        // Assume use same speed which is 6 px per frame
        float dX = OppositionX - posX;
        float dY = OppositionY - posY;
        if(dX==0 && dY==0){ // kills this fx once arrives to location
            currentState = Knocking;
            return;
        }
        RecallAttack.push_back({posX,posY}); // record position of X and Y
        float ratioX =0;
        float ratioY = 0;
        NormalizedHypotenous = sqrt((dX * dX)+(dY * dY));
        ratioX = dX/NormalizedHypotenous;
        ratioY = dY/NormalizedHypotenous;
        vX = (SPEED * deltaTime) * ratioX;
        vY = (SPEED * deltaTime) * ratioY;

        // Check if already arrived or not to prevent wall bug
        if (NormalizedHypotenous <= SPEED * deltaTime){
            posX = OppositionX;
            posY = OppositionY;

        }
        else{
        posX += vX;
        posY += vY;
        }
    }