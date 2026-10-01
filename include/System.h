#pragma once
#include <Game.h>

void S_MovementX(void);
void S_CollisionX(uint16_t LastTile, uint16_t LastEnemy);
void S_MovementY(uint16_t LastEnemy);
void S_CollisionY(uint16_t LastTile, uint16_t LastEnemy);
void S_Camera(void);
void S_Animation(void);
void S_Gravity(uint16_t LastGravity);
