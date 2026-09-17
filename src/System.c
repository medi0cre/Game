#include <System.h>
#include <Assets.h>
#include <Utils.h>
#include <Component.h>

void S_Gravity(uint16_t* TempGravityArray, uint16_t LastGravity)
{
    Enforce(TempGravityArray && LastGravity < EntityMax, "Precondition broken inside S_Gravity()");

    for (uint16_t i = 0; i < LastGravity; i++)
    {
        uint16_t ID = TempGravityArray[i];
        Enforce(ID < EntityMax
            && Gravities[ID].Acceleration > 0.0f
            && Movements[ID].Velocity.y <= Gravities[ID].MaxVelocity && Movements[ID].Velocity.y >= -Gravities[ID].MaxVelocity
            , "Invalid values inside S_Gravity()");

        Movements[ID].Velocity.y += Gravities[ID].Acceleration;
        if (Movements[ID].Velocity.y > Gravities[ID].MaxVelocity) { Movements[ID].Velocity.y = Gravities[ID].MaxVelocity; }
    }
}

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
    Gravities[PlayerID].Grounded = false;

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
        Enforce(Overlap.width > 0.0f && Overlap.height > 0.0f, "Buggy overlap");

        // TODO: Find a better way to handle this. Consider using previous position to check if there was a new collision
        // Enforce(Overlap.width != Overlap.height, "Exact same collision resolution between object and player");

        if (Overlap.width >= Overlap.height) // Vertical Collision
        {
            Enforce(Spatials[PlayerID].Position.y != Spatials[ID].Position.y, "Exact same y position");
            //Enforce(Movements[PlayerID].y != 0.0f, "Invariant broken inside S_Collision()");

            // Player hits tile from below
            if (Movements[PlayerID].Velocity.y < 0.0f)
            {
                Spatials[PlayerID].Position.y += Overlap.height;
                Movements[PlayerID].Velocity.y = 0.0f;
            }

            // Player hits tile from above
            else if (Movements[PlayerID].Velocity.y > 0.0f)
            {
                Spatials[PlayerID].Position.y -= Overlap.height;
                Movements[PlayerID].Velocity.y = 0.0f;
                Gravities[PlayerID].Grounded = true;
            }

        }
        else // Horizontal Collision
        {
            Enforce(Spatials[PlayerID].Position.x != Spatials[ID].Position.x, "Exact same x position");

            // Player hits tile from the right
            if (Movements[PlayerID].Velocity.x < 0.0f)
            {
                Spatials[PlayerID].Position.x += Overlap.width;
                Movements[PlayerID].Velocity.x = 0.0f;
            }

            // Player hits tile from the left
            else if (Movements[PlayerID].Velocity.x > 0.0f)
            {
                Spatials[PlayerID].Position.x -= Overlap.width;
                Movements[PlayerID].Velocity.x = 0.0f;
            }
        }
    }
}

void S_Movement(Input UserInput)
{
    // Player
    if (UserInput.Right && !UserInput.Left)
    {
        Movements[PlayerID].Velocity.x = (float)PlayerSpeed;
        Movements[PlayerID].Direction.x = 1.0f;
        //Spatials[PlayerID].Position.x += Movements[PlayerID].x;

        Animations[PlayerID].AnimationID = SamuraiWalk;
        Animations[PlayerID].FramesInAnimation = FrameCountSamuraiWalk;
        Animations[PlayerID].Duration = FrameDurationSamuraiWalk;
    }
    else if (!UserInput.Right && UserInput.Left)
    {
        Movements[PlayerID].Velocity.x = -(float)PlayerSpeed;
        Movements[PlayerID].Direction.x = -1.0f;
        //Spatials[PlayerID].Position.x -= Movements[PlayerID].x;

        Animations[PlayerID].AnimationID = SamuraiWalk;
        Animations[PlayerID].FramesInAnimation = FrameCountSamuraiWalk;
        Animations[PlayerID].Duration = FrameDurationSamuraiWalk;
    }
    else
    {
        Movements[PlayerID].Velocity.x = 0.0f;

        Animations[PlayerID].AnimationID = SamuraiIdle;
        Animations[PlayerID].FramesInAnimation = FrameCountSamuraiIdle;
        Animations[PlayerID].Duration = FrameDurationSamuraiIdle;
    }

    if (UserInput.Up && Gravities[PlayerID].Grounded)
    {
        Movements[PlayerID].Velocity.y -= Gravities[PlayerID].Jump;
        if (Movements[PlayerID].Velocity.y < -Gravities[PlayerID].MaxVelocity) { Movements[PlayerID].Velocity.y = -Gravities[PlayerID].MaxVelocity; }
    }

    if (!UserInput.Up && !Gravities[PlayerID].Grounded && Movements[PlayerID].Velocity.y < 0.0f) { Movements[PlayerID].Velocity.y = 0.0f; }

    if (!Gravities[PlayerID].Grounded)
    {
        Animations[PlayerID].AnimationID = SamuraiJump;
        Animations[PlayerID].FramesInAnimation = FrameCountSamuraiJump;
        Animations[PlayerID].Duration = FrameDurationSamuraiJump;
    }

    Spatials[PlayerID].Position.y += Movements[PlayerID].Velocity.y;
    Spatials[PlayerID].Position.x += Movements[PlayerID].Velocity.x;
}
