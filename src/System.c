#include <System.h>
#include <Assets.h>
#include <Utils.h>
#include <Component.h>
#include <math.h>

void S_Gravity(uint16_t LastGravity)
{
    Enforce(LastGravity < MaxEntityCount, "Precondition broken inside S_Gravity()");
    World* W = &CurrentGame.GameWorld;

    for (uint16_t i = 0; i < LastGravity; i++)
    {
        uint16_t ID = W->TempGravityArray[i];

        Enforce(ID < MaxEntityCount
            && W->Gravities[ID].Acceleration > 0.0f
            && W->Movements[ID].Magnitude.y <= W->Gravities[ID].MaxVelocity
            && W->Movements[ID].Magnitude.y >= 0.0f
            && (W->Movements[ID].Direction.y == 1.0f || W->Movements[ID].Direction.y == -1.0f)
            , "Invalid values inside S_Gravity()");

        float Velocity = W->Movements[ID].Magnitude.y * W->Movements[ID].Direction.y;
        Velocity += W->Gravities[ID].Acceleration;

        Velocity = fminf(Velocity, W->Gravities[ID].MaxVelocity);
        W->Movements[ID].Magnitude.y = fabsf(Velocity);
        W->Movements[ID].Direction.y = Velocity >= 0.0f ? 1.0f : -1.0f;
    }
}

void S_Animation(void)
{
    World* W = &CurrentGame.GameWorld;
    Enforce(W->TempAnimationArray
        && W->LastAnimation < MaxEntityCount
        , "Precondition broken inside S_Animation()");

    for (uint16_t i = 0; i < W->LastAnimation; i++)
    {
        uint16_t ID = W->TempAnimationArray[i];
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
    World* W = &CurrentGame.GameWorld;
    W->Gravities[PID].Grounded = false;

    for (uint16_t i = 0; i < LastCollisionBox; i++)
    {
        uint16_t ID = W->TempCollisionBoxArray[i];
        if (ID == PID) { continue; }

        Enforce(ID < MaxEntityCount, "Invalid ID inside S_Collision()");

        Rectangle PlayerBox = {
            .x = W->Spatials[PID].Position.x,
            .y = W->Spatials[PID].Position.y,
            .width = W->CollisionBoxes[PID].Width,
            .height = W->CollisionBoxes[PID].Height
        };

        Rectangle TileBox = {
            .x = W->Spatials[ID].Position.x,
            .y = W->Spatials[ID].Position.y,
            .width = W->CollisionBoxes[ID].Width,
            .height = W->CollisionBoxes[ID].Height
        };

        Rectangle Overlap = GetCollisionRec(PlayerBox, TileBox);
        if (Overlap.x == 0.0f && Overlap.y == 0.0f) { continue; }
        Enforce(Overlap.height != Overlap.width, "Same collision dimensions cannot be resolved, HAAALP!!");

        if (Overlap.width > Overlap.height) // Vertical Collision
        {
            // Player hits tile from below
            if (W->Spatials[PID].Position.y < W->Movements[PID].PreviousPosition.y)
            {
                W->Spatials[PID].Position.y += Overlap.height;
                W->Movements[PID].Magnitude.y = 0.0f;
                W->Movements[PID].Direction.y = 1.0f;
            }

            // Player hits tile from above
            else if (W->Spatials[PID].Position.y > W->Movements[PID].PreviousPosition.y)
            {
                W->Spatials[PID].Position.y -= Overlap.height;
                W->Movements[PID].Magnitude.y = 0.0f;
                W->Movements[PID].Direction.y = 1.0f;
                W->Gravities[PID].Grounded = true;
            }

        }
        else if (Overlap.width < Overlap.height) // Horizontal Collision
        {
            // Player hits tile from the right
            if (W->Spatials[PID].Position.x < W->Movements[PID].PreviousPosition.x)
            {
                W->Spatials[PID].Position.x += Overlap.width;
                W->Movements[PID].Magnitude.x = 0.0f;
            }

            // Player hits tile from the left
            else if (W->Spatials[PID].Position.x > W->Movements[PID].PreviousPosition.x)
            {
                W->Spatials[PID].Position.x -= Overlap.width;
                W->Movements[PID].Magnitude.x = 0.0f;
            }
        }
    }
}

void S_Movement(void)
{
    uint16_t PID = CurrentGame.PlayerID;
    Input UserInput = CurrentGame.UserInput;
    World* W = &CurrentGame.GameWorld;

    Enforce(PID < MaxEntityCount, "Invalid Player ID");

    if (UserInput.Right && !UserInput.Left)
    {
        W->Movements[PID].Magnitude.x = (float)PlayerSpeed;
        W->Movements[PID].Direction.x = 1.0f;

        W->Animations[PID].AnimationID = SamuraiRun;
        W->Animations[PID].FramesInAnimation = FrameCountSamuraiRun;
        W->Animations[PID].Duration = FrameDurationSamuraiRun;
    }
    else if (!UserInput.Right && UserInput.Left)
    {
        W->Movements[PID].Magnitude.x = (float)PlayerSpeed;
        W->Movements[PID].Direction.x = -1.0f;

        W->Animations[PID].AnimationID = SamuraiRun;
        W->Animations[PID].FramesInAnimation = FrameCountSamuraiRun;
        W->Animations[PID].Duration = FrameDurationSamuraiRun;
    }
    else
    {
        W->Movements[PID].Magnitude.x = 0.0f;

        W->Animations[PID].AnimationID = SamuraiIdle;
        W->Animations[PID].FramesInAnimation = FrameCountSamuraiIdle;
        W->Animations[PID].Duration = FrameDurationSamuraiIdle;
    }

    if (UserInput.Up && W->Gravities[PID].Grounded)
    {
        float Velocity = W->Movements[PID].Magnitude.y * W->Movements[PID].Direction.y;
        Velocity = -W->Gravities[PID].Jump;
        Velocity = fmaxf(Velocity, -W->Gravities[PID].MaxVelocity);

        W->Movements[PID].Magnitude.y = fabsf(Velocity);
        W->Movements[PID].Direction.y = Velocity >= 0.0f ? 1.0f : -1.0f;
    }

    if (!UserInput.Up && !W->Gravities[PID].Grounded
        && W->Movements[PID].Magnitude.y
        * W->Movements[PID].Direction.y < 0.0f)
    {
        W->Movements[PID].Magnitude.y = 0.0f;
        W->Movements[PID].Direction.y = 1.0f;
    }

    if (!W->Gravities[PID].Grounded)
    {
        W->Animations[PID].AnimationID = SamuraiJump;
        W->Animations[PID].FramesInAnimation = FrameCountSamuraiJump;
        W->Animations[PID].Duration = FrameDurationSamuraiJump;
    }

    W->Movements[PID].PreviousPosition = W->Spatials[PID].Position;
    W->Spatials[PID].Position.y += W->Movements[PID].Magnitude.y * W->Movements[PID].Direction.y;
    W->Spatials[PID].Position.x += W->Movements[PID].Magnitude.x * W->Movements[PID].Direction.x;
}
