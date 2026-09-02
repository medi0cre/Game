#pragma once
#include <raylib.h>

#define AnimationCount 30

extern Texture AnimationArray[AnimationCount];

typedef enum {
    FighterAttack1,
    FighterAttack2,
    FighterAttack3,
    FighterDead,
    FighterHurt,
    FighterIdle,
    FighterJump,
    FighterRun,
    FighterShield,
    FighterWalk,
    SamuraiAttack1,
    SamuraiAttack2,
    SamuraiAttack3,
    SamuraiDead,
    SamuraiHurt,
    SamuraiIdle,
    SamuraiJump,
    SamuraiRun,
    SamuraiShield,
    SamuraiWalk,
    ShinobiAttack1,
    ShinobiAttack2,
    ShinobiAttack3,
    ShinobiDead,
    ShinobiHurt,
    ShinobiIdle,
    ShinobiJump,
    ShinobiRun,
    ShinobiShield,
    ShinobiWalk
} AnimationIndex;

