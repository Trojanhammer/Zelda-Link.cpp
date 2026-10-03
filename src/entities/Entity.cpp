#include "Entity.h"
#include "CombatUtils.h"

    void Entity::Return(std::vector<RecallPoint>& Recall, float deltaTime){
        if(RecallClock <0){
            RecallClock = Recall.back().time;
        }
        RecallClock -= deltaTime;
        int i;

        // (98, 48, 2.5) -> (99,49,2.67) -> (100,50, 3) ->(x,y, 2.98) -> (x,y,2.5) -> (x,y,0)
        //(99,49, 2.67) -> (x,y,2.6), (x,y,2.59) rati0 =0.8
        
        for (i = Recall.size() -1 ; i>0; i--){
            if (Recall[i].time == RecallClock){
                posX = Recall[i].posX;
                posY = Recall[i].posY;
                break;
            }
            else if (Recall[i].time > RecallClock && Recall[i-1].time < RecallClock){ 
                float b = RecallClock - Recall[i-1].time;
                float c = Recall[i].time - Recall[i-1].time;
                float ratio = b/c;
                posX = Recall[i-1].posX + (ratio * (Recall[i].posX - Recall[i-1].posX));
                posY = Recall[i-1].posY + (ratio * (Recall[i].posY - Recall[i-1].posY));
                break;
            }

        }
        if(RecallClock <= Recall[0].time){
            posX = basePosX;
            posY  = basePosY;
            RecallClock = -1.0f; // Set back the to enable next round for Recalling
            currentState = Idle;
            time =0;
            Recall.clear();
            return;
        }
    }

    void Entity::KnockBack(float VxOpponent, float VyOpponent){
        vX = VxOpponent;
        vY = VyOpponent;
    }
    void Entity::UpdateKnock(float deltaTime){
        if(Recall_After_Knocked.empty()){
            Recall_After_Knocked.push_back({(float)basePosX,(float)basePosY,0.0f}); // record the base position of X and Y
        }
        vX *= std::pow(0.90, deltaTime * 60.0f); // multiply by 60 to make it frame rate independent
        vY *= std::pow(0.90, deltaTime * 60.0f); 
        // alternative way to make it frame rate independent is to use std::pow(0.90, deltaTime * 60) instead of 0.90
        if(std::abs(vX) <6.0f && std::abs(vY) <6.0f){ // 6 px/sec, a stable speed threshold (the old 0.1px/frame at 60fps); checking displacement (vX*deltaTime) instead falsely reports "stopped" at full speed when deltaTime is very small
            vX=0;
            vY=0;
            currentState = Recalling_After_Knocked;
            return;
        }
        time += deltaTime;
        posX += vX * deltaTime;
        posY += vY * deltaTime;
        Recall_After_Knocked.push_back({posX,posY,time});
    }

    void Entity::TakeDamage(uint8_t damage){  
        if(!isAlive){
            return; //return is for exit the function
        };

        ApplyDamage(health,damage,isAlive);
    }

    void Entity::Attack(float OppositionX, float OppositionY,float deltaTime){
        if(Recall_After_Attack.empty()){
            Recall_After_Attack.push_back({(float)basePosX,(float)basePosY,0.0f}); // record the base position of X and Y
        }

        // set capped at 3000px/s
        // Assume use same speed which is 6 px per frame
        float dX = OppositionX - posX;
        float dY = OppositionY - posY;
        if(dX==0 && dY==0){ // kills this fx once arrives to location
            currentState = Knocking;
            return;
        }

        time += deltaTime;
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
        Recall_After_Attack.push_back({posX,posY,time}); // record position of X and Y
    }