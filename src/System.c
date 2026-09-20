#include <System.h>
#include <Assets.h>
#include <Utils.h>
#include <Component.h>
#include <math.h>

void S_Gravity(uint16_t LastGravity)
{
    Enforce(LastGravity < MaxEntityCount, "Precondition broken inside S_Gravity()");

    for (uint16_t i = 0; i < LastGravity; i++)
    {
        uint16_t ID = CurrentGame.GameWorld.TempGravityArray[i];
        Enforce(ID < MaxEntityCount
            && CurrentGame.GameWorld.Gravities[ID].Acceleration > 0.0f
            && CurrentGame.GameWorld.Movements[ID].Magnitude.y <= CurrentGame.GameWorld.Gravities[ID].MaxVelocity
            && CurrentGame.GameWorld.Movements[ID].Magnitude.y >= 0.0f
            && (CurrentGame.GameWorld.Movements[ID].Direction.y == 1.0f || CurrentGame.GameWorld.Movements[ID].Direction.y == -1.0f)
            , "Invalid values inside S_Gravity()");

        float Velocity = CurrentGame.GameWorld.Movements[ID].Magnitude.y * CurrentGame.GameWorld.Movements[ID].Direction.y;
        Velocity += CurrentGame.GameWorld.Gravities[ID].Acceleration;

        Velocity = fminf(Velocity, CurrentGame.GameWorld.Gravities[ID].MaxVelocity);
        CurrentGame.GameWorld.Movements[ID].Magnitude.y = fabsf(Velocity);
        CurrentGame.GameWorld.Movements[ID].Direction.y = Velocity >= 0.0f ? 1.0f : -1.0f;
    }
}

void S_Animation(void)
{
    Enforce(CurrentGame.GameWorld.TempAnimationArray
        && CurrentGame.GameWorld.LastAnimation < MaxEntityCount
        , "Precondition broken inside S_Animation()");

    for (uint16_t i = 0; i < CurrentGame.GameWorld.LastAnimation; i++)
    {
        uint16_t ID = CurrentGame.GameWorld.TempAnimationArray[i];
        Enforce(ID < MaxEntityCount, "Invalid ID inside S_Animation()");
        Animation* A = &CurrentGame.GameWorld.Animations[ID];

        if (A->FramesPassed > A->Duration)
        {
            A->CurrentFrameInAnimation = (A->CurrentFrameInAnimation + 1) % A->FramesInAnimation;
            A->FramesPassed = 0;
            continue;
        }

        A->FramesPassed++;
    }
}

void S_Collision(uint16_t LastCollisionBox)
{
    Enforce(LastCollisionBox < MaxEntityCount, "Precondition broken inside S_Collision()");

    uint16_t PID = CurrentGame.PlayerID;
    CurrentGame.GameWorld.Gravities[PID].Grounded = false;

    for (uint16_t i = 0; i < LastCollisionBox; i++)
    {
        uint16_t ID = CurrentGame.GameWorld.TempCollisionBoxArray[i];
        if (ID == PID) { continue; }

        Enforce(ID < MaxEntityCount, "Invalid ID inside S_Collision()");

        Rectangle PlayerBox = {
            .x = CurrentGame.GameWorld.Spatials[PID].Position.x,
            .y = CurrentGame.GameWorld.Spatials[PID].Position.y,
            .width = CurrentGame.GameWorld.CollisionBoxes[PID].Width,
            .height = CurrentGame.GameWorld.CollisionBoxes[PID].Height
        };

        Rectangle TileBox = {
            .x = CurrentGame.GameWorld.Spatials[ID].Position.x,
            .y = CurrentGame.GameWorld.Spatials[ID].Position.y,
            .width = CurrentGame.GameWorld.CollisionBoxes[ID].Width,
            .height = CurrentGame.GameWorld.CollisionBoxes[ID].Height
        };

        Rectangle Overlap = GetCollisionRec(PlayerBox, TileBox);
        if (Overlap.width == 0.0f && Overlap.height == 0.0f) { continue; }
        Enforce(Overlap.width > 0.0f && Overlap.height > 0.0f, "Buggy overlap");

        // TODO: Find a better way to handle this. Consider using previous position to check if there was a new collision

        if (Overlap.width >= Overlap.height) // Vertical Collision
        {
            Enforce(CurrentGame.GameWorld.Spatials[PID].Position.y != CurrentGame.GameWorld.Spatials[ID].Position.y
                , "Exact same y position");

            float Velocity = CurrentGame.GameWorld.Movements[PID].Magnitude.y * CurrentGame.GameWorld.Movements[PID].Direction.y;

            // Player hits tile from below
            if (Velocity < 0.0f)
            {
                CurrentGame.GameWorld.Spatials[PID].Position.y += Overlap.height;
                CurrentGame.GameWorld.Movements[PID].Magnitude.y = 0.0f;
                CurrentGame.GameWorld.Movements[PID].Direction.y = 1.0f;
            }

            // Player hits tile from above
            else if (Velocity > 0.0f)
            {
                CurrentGame.GameWorld.Spatials[PID].Position.y -= Overlap.height;
                CurrentGame.GameWorld.Movements[PID].Magnitude.y = 0.0f;
                CurrentGame.GameWorld.Movements[PID].Direction.y = 1.0f;
                CurrentGame.GameWorld.Gravities[PID].Grounded = true;
            }

        }
        else // Horizontal Collision
        {
            Enforce(CurrentGame.GameWorld.Spatials[PID].Position.x != CurrentGame.GameWorld.Spatials[ID].Position.x
                , "Exact same x position");

            float Velocity = CurrentGame.GameWorld.Movements[PID].Magnitude.x * CurrentGame.GameWorld.Movements[PID].Direction.x;

            // Player hits tile from the right
            if (Velocity < 0.0f)
            {
                CurrentGame.GameWorld.Spatials[PID].Position.x += Overlap.width;
                CurrentGame.GameWorld.Movements[PID].Magnitude.x = 0.0f;
            }

            // Player hits tile from the left
            else if (Velocity > 0.0f)
            {
                CurrentGame.GameWorld.Spatials[PID].Position.x -= Overlap.width;
                CurrentGame.GameWorld.Movements[PID].Magnitude.x = 0.0f;
            }
        }
    }
}

void S_Movement(void)
{
    uint16_t PID = CurrentGame.PlayerID;
    Input UserInput = CurrentGame.UserInput;
    Enforce(PID < MaxEntityCount, "Invalid Player ID");

    if (UserInput.Right && !UserInput.Left)
    {
        CurrentGame.GameWorld.Movements[PID].Magnitude.x = (float)PlayerSpeed;
        CurrentGame.GameWorld.Movements[PID].Direction.x = 1.0f;

        CurrentGame.GameWorld.Animations[PID].AnimationID = SamuraiRun;
        CurrentGame.GameWorld.Animations[PID].FramesInAnimation = FrameCountSamuraiRun;
        CurrentGame.GameWorld.Animations[PID].Duration = FrameDurationSamuraiRun;
    }
    else if (!UserInput.Right && UserInput.Left)
    {
        CurrentGame.GameWorld.Movements[PID].Magnitude.x = (float)PlayerSpeed;
        CurrentGame.GameWorld.Movements[PID].Direction.x = -1.0f;

        CurrentGame.GameWorld.Animations[PID].AnimationID = SamuraiRun;
        CurrentGame.GameWorld.Animations[PID].FramesInAnimation = FrameCountSamuraiRun;
        CurrentGame.GameWorld.Animations[PID].Duration = FrameDurationSamuraiRun;
    }
    else
    {
        CurrentGame.GameWorld.Movements[PID].Magnitude.x = 0.0f;

        CurrentGame.GameWorld.Animations[PID].AnimationID = SamuraiIdle;
        CurrentGame.GameWorld.Animations[PID].FramesInAnimation = FrameCountSamuraiIdle;
        CurrentGame.GameWorld.Animations[PID].Duration = FrameDurationSamuraiIdle;
    }

    if (UserInput.Up && CurrentGame.GameWorld.Gravities[PID].Grounded)
    {
        float Velocity = CurrentGame.GameWorld.Movements[PID].Magnitude.y * CurrentGame.GameWorld.Movements[PID].Direction.y;
        Velocity = -CurrentGame.GameWorld.Gravities[PID].Jump;
        Velocity = fmaxf(Velocity, -CurrentGame.GameWorld.Gravities[PID].MaxVelocity);

        CurrentGame.GameWorld.Movements[PID].Magnitude.y = fabsf(Velocity);
        CurrentGame.GameWorld.Movements[PID].Direction.y = Velocity >= 0.0f ? 1.0f : -1.0f;
    }

    if (!UserInput.Up && !CurrentGame.GameWorld.Gravities[PID].Grounded
        && CurrentGame.GameWorld.Movements[PID].Magnitude.y
        * CurrentGame.GameWorld.Movements[PID].Direction.y < 0.0f)
    {
        CurrentGame.GameWorld.Movements[PID].Magnitude.y = 0.0f;
        CurrentGame.GameWorld.Movements[PID].Direction.y = 1.0f;
    }

    if (!CurrentGame.GameWorld.Gravities[PID].Grounded)
    {
        CurrentGame.GameWorld.Animations[PID].AnimationID = SamuraiJump;
        CurrentGame.GameWorld.Animations[PID].FramesInAnimation = FrameCountSamuraiJump;
        CurrentGame.GameWorld.Animations[PID].Duration = FrameDurationSamuraiJump;
    }

    CurrentGame.GameWorld.Spatials[PID].Position.y += CurrentGame.GameWorld.Movements[PID].Magnitude.y * CurrentGame.GameWorld.Movements[PID].Direction.y;
    CurrentGame.GameWorld.Spatials[PID].Position.x += CurrentGame.GameWorld.Movements[PID].Magnitude.x * CurrentGame.GameWorld.Movements[PID].Direction.x;
}
