#include <System.h>
#include <Utils.h>
#include <Component.h>

void S_Animation(uint16_t* TempAnimationArray, uint16_t LastAnimation)
{
    Enforce(TempAnimationArray && LastAnimation < EntityMax, "Precondition broken inside S_Animation()");

    for (uint16_t i = 0; i < LastAnimation; i++)
    {
        uint16_t ID = TempAnimationArray[i];
        Enforce(ID < EntityMax, "Invalid ID inside S_Animation()");

        if (Animations[ID].FramesPassed > Animations[ID].Duration)
        {
            Animations[ID].CurrentFrameInAnimation = (Animations[ID].CurrentFrameInAnimation + 1) % Animations[ID].FramesInAnimation;
            Animations[ID].FramesPassed = 0;
        }

        Animations[ID].FramesPassed++;
    }
}

void S_Collision(uint16_t* TempCollisionBoxArray, uint16_t LastCollisionBox)
{
    Enforce(TempCollisionBoxArray && LastCollisionBox < EntityMax, "Precondition broken inside S_Collision()");

    for (uint16_t i = 0; i < LastCollisionBox; i++)
    {
        uint16_t ID = TempCollisionBoxArray[i];
        if (ID == PlayerID) { continue; }

        Enforce(ID < EntityMax, "Invalid ID inside S_Collision()");

        Rectangle PlayerBox = {
            .x = Spatials[PlayerID].Position.x,
            .y = Spatials[PlayerID].Position.y,
            .width = CollisionBoxes[PlayerID].Width,
            .height = CollisionBoxes[PlayerID].Height
        };

        Rectangle TileBox = {
            .x = Spatials[ID].Position.x,
            .y = Spatials[ID].Position.y,
            .width = CollisionBoxes[ID].Width,
            .height = CollisionBoxes[ID].Height
        };

        Rectangle Overlap = GetCollisionRec(PlayerBox, TileBox);
        if (Overlap.width == 0.0f && Overlap.height == 0.0f) { continue; }

        // TODO: Find a better way to handle this. Consider using previous position to check if there was a new collision
        // Enforce(Overlap.width != Overlap.height, "Exact same collision resolution between object and player");

        if (Overlap.width >= Overlap.height) // Vertical Collision
        {
            Enforce(Spatials[PlayerID].Position.y != Spatials[ID].Position.y, "Exact same y position");

            // Player hits tile from below
            if (Spatials[PlayerID].Position.y > Spatials[ID].Position.y) { Spatials[PlayerID].Position.y += Overlap.height; }

            // Player hits tile from above
            else { Spatials[PlayerID].Position.y -= Overlap.height; }
        }
        else // Horizontal Collision
        {
            Enforce(Spatials[PlayerID].Position.x != Spatials[ID].Position.x, "Exact same x position");

            // Player hits tile from the right
            if (Spatials[PlayerID].Position.x > Spatials[ID].Position.x) { Spatials[PlayerID].Position.x += Overlap.width; }

            // Player hits tile from the left
            else { Spatials[PlayerID].Position.x -= Overlap.width; }
        }
    }
}

void S_Movement(Input UserInput)
{
    // Player
    if (UserInput.Up) { Spatials[PlayerID].Position.y -= Movements[PlayerID].Velocity; }
    if (UserInput.Down) { Spatials[PlayerID].Position.y += Movements[PlayerID].Velocity; }
    if (UserInput.Right) { Spatials[PlayerID].Position.x += Movements[PlayerID].Velocity; }
    if (UserInput.Left) { Spatials[PlayerID].Position.x -= Movements[PlayerID].Velocity; }
}
