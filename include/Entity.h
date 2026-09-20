#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    Player = 0,
    Tile = 1,
    Decoration = 2,
} EntityType;

uint16_t CreateEntity(void);
void DestroyEntity(uint16_t Entity);
