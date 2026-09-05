#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ArenaMax 268435456 // 256 Megabytes

typedef struct {
    unsigned char* Start;
    unsigned char* Current;
    unsigned char* Snapshot;
    size_t Size;
} Arena;

bool ArenaInit(Arena* _Arena_, size_t Size);
void* ArenaAlloc(Arena* _Arena_, size_t Size, size_t Alignment);
bool ArenaSnapshot(Arena* _Arena_);
bool ArenaReset(Arena* _Arena_);
bool ArenaResetToSnapshot(Arena* _Arena_);
bool ArenaFree(Arena* _Arena_);
