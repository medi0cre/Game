#pragma once
#include <stdbool.h>

typedef struct {
    bool Z;
    bool X;
    bool C;
    bool Up;
    bool Down;
    bool Left;
    bool Right;
} Input;

Input GetUserInput(void);
