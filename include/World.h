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
    bool* Actives;

    // Temporary Resources
    uint16_t* TempSpatialArray;
    uint16_t* TempMovementArray;
    uint16_t* TempGravityArray;
    uint16_t* TempTextureArray;
    uint16_t* TempAnimationArray;
    uint16_t* TempCollisionBoxArray;

    uint16_t LastTexture;
    uint16_t LastAnimation;
} World;
