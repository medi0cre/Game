#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    Tile = 0,
    Decoration = 1,
    Enemy = 2
} EntityType;

uint16_t CreateEntity(void);
void DestroyEntity(uint16_t Entity);
