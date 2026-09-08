#include <System.h>
#include <Component.h>

void S_Movement(Input UserInput)
{
    // Player
    if (UserInput.Up) { Spatials[PlayerID].Position.y -= Movements[PlayerID].Velocity; }
    if (UserInput.Down) { Spatials[PlayerID].Position.y += Movements[PlayerID].Velocity; }
    if (UserInput.Right) { Spatials[PlayerID].Position.x += Movements[PlayerID].Velocity; }
    if (UserInput.Left) { Spatials[PlayerID].Position.x -= Movements[PlayerID].Velocity; }
}
