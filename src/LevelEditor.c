#include <LevelEditor.h>
#include <raylib.h>

// Raygui has many unused variables and functions in its implementation
// This is just a way to remove warnings caused by those variables and functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#pragma GCC diagnostic pop
