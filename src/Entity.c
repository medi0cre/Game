#include <Entity.h>
#include <Game.h>
#include <Utils.h>

uint16_t CreateEntity(void)
{
    for (uint16_t i = 0; i < MaxEntityCount; i++)
    {
        if (!CurrentGame.GameWorld.Actives[i])
        {
            CurrentGame.GameWorld.Actives[i] = true;
            CurrentGame.GameWorld.Components[i] = 0;
            return i;
        }
    }

    return UINT16_MAX;
}

void DestroyEntity(uint16_t Entity)
{
    Enforce(Entity < MaxEntityCount && CurrentGame.GameWorld.Actives[Entity], "DestroyEntity() error");
    CurrentGame.GameWorld.Actives[Entity] = false;
    CurrentGame.GameWorld.Components[Entity] = 0;
}
