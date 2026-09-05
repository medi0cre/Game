#pragma once
#include <stdint.h>
#include <stdbool.h>

#define EntityMax 10000

typedef struct {
    uint64_t Components;
    bool Active;
} Entity;

extern Entity* Entities;

uint16_t CreateEntity(void);
void DestroyEntity(uint16_t _Entity_);
