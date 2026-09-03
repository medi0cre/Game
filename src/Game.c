#include <Game.h>
#include <raylib.h>

Input GetUserInput(void)
{
    return (Input) {
        .Z = IsKeyDown(KEY_Z),
        .X = IsKeyDown(KEY_X),
        .C = IsKeyDown(KEY_C),
        .Up = IsKeyDown(KEY_UP),
        .Down = IsKeyDown(KEY_DOWN),
        .Left = IsKeyDown(KEY_LEFT),
        .Right = IsKeyDown(KEY_RIGHT)
    };
}
