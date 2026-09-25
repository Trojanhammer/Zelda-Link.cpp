#include "Entity.h"
#include "CombatUtils.h"

    void Entity::Return(std::vector<std::pair<float, float>>& Recall){
        // read the pair from backwards and set to posX and posY
        posX = Recall.back().first;
        posY = Recall.back().second;

        Recall.pop_back(); // Delete last pair 

        if(posX == basePosX && posY == basePosY){
            isReturn = false;
            isKnocked = false;
            ReturnAfterKnocked = false;
        }
    }

    void Entity::KnockBack(float VxOpponent, float VyOpponent){
        vX = VxOpponent;
        vY = VyOpponent;
        isKnocked = true;
    }
    void Entity::UpdateKnock(){
        if(ReturnAfterKnocked){
            Return(RecallKnocked);
            return;
        }
        vX *= 0.90;
        vY *= 0.90;
        if(std::abs(vX) <0.1 && std::abs(vY) <0.1){
            vX=0;
            vY=0;
            ReturnAfterKnocked = true;
            return;
        }
        RecallKnocked.push_back({posX, posY});
        posX += vX;
        posY += vY;
    }

    void Entity::TakeDamage(uint8_t damage){  
        if(!isAlive){
            return; //return is for exit the function
        };

        ApplyDamage(health,damage,isAlive);
    }