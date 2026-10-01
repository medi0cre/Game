#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <Component.h>

typedef struct {
    Spatial* Spatials;
    Movement* Movements;
    Gravity* Gravities;
    CollisionBox* CollisionBoxes;
    uint64_t* Components;
    uint16_t* Textures;
    Animation* Animations;
    Health* Healths;
    bool* Actives;

    // Temporary Resources
    uint16_t* TempGravityArray;
    uint16_t* TempTextureArray;
    uint16_t* TempAnimationArray;
    uint16_t* TempTileArray;
    uint16_t* TempEnemyArray;

    uint16_t LastTexture;
    uint16_t LastAnimation;
} World;
