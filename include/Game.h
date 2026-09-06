#pragma once
#include <stdbool.h>
#include <Arena.h>

typedef struct {
    bool Z;
    bool X;
    bool C;
    bool Up;
    bool Down;
    bool Left;
    bool Right;
} Input;

extern Arena GameArena;

void GameInit(void);
Input GetUserInput(void);
void LoadLevel(const char* File);
void Render(void);
//void Update(Input _Input_);
