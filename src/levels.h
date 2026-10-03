#pragma once
#include "Enemy.h"
#include <vector>


struct BlueprintLevel {
    Enemy::EnemyStats BokoData;
    Enemy::EnemyStats StalfosData;
};

const std::vector<BlueprintLevel> LevelStats {
    {   // level_0
        {.name = "Bokobolin", .health = 80, .maxHealth = 80, .attackPower = 10, .posX = 150, .posY = 100, .basePosX = 150, .basePosY = 100, .vX = 0, .vY = 0},
        {.name = "Stalfos", .health = 80, .maxHealth = 80, .attackPower = 20, .posX = 550, .posY = 160, .basePosX = 550, .basePosY = 160, .vX = 0, .vY = 0}
    },
    {   // level_1
        {.name = "Bokobolin", .health = 80, .maxHealth = 80, .attackPower = 10, .posX = 250, .posY = 200, .basePosX = 250, .basePosY = 200, .vX = 0, .vY = 0},
        {.name = "Stalfos", .health = 80, .maxHealth = 80, .attackPower = 20, .posX = 600, .posY = 240, .basePosX = 600, .basePosY = 240, .vX = 0, .vY = 0},
    },
    {   // level_2
        {.name = "Bokobolin", .health = 110, .maxHealth = 110, .attackPower = 25, .posX = 300, .posY = 200, .basePosX = 300, .basePosY = 200, .vX = 0, .vY = 0},
        {.name = "Stalfos", .health = 120, .maxHealth = 120, .attackPower = 35, .posX = 350, .posY = 240, .basePosX = 350, .basePosY = 240, .vX = 0, .vY = 0}
    }
};