#pragma once
#include <Game.h>

void S_Movement(Input UserInput);
void S_Collision(uint16_t* TempCollisionBoxArray, uint16_t LastCollisionBox);
void S_Animation(uint16_t* TempAnimationArray, uint16_t LastAnimation);
void S_Gravity(uint16_t* TempGravityArray, uint16_t LastGravity);
