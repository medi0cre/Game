#pragma once
#include <stdint.h>
#include <stdbool.h>

#define EntityMax 10000

typedef struct {
    uint64_t ComponentMask;
    bool Active;
} Entity;

typedef enum {
    Player = 0,
    Tile = 1,
    Decoration = 2,
} EntityType;

extern Entity* Entities;

uint16_t CreateEntity(void);
void DestroyEntity(uint16_t _Entity_);
