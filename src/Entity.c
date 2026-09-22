#include <Entity.h>
#include <Game.h>
#include <Utils.h>

uint16_t CreateEntity(void)
{
    World* W = &CurrentGame.GameWorld;

    for (uint16_t i = 0; i < MaxEntityCount; i++)
    {
        if (!W->Actives[i])
        {
            W->Actives[i] = true;
            W->Components[i] = 0;
            return i;
        }
    }

    return UINT16_MAX;
}

void DestroyEntity(uint16_t Entity)
{
    World* W = &CurrentGame.GameWorld;
    Enforce(Entity < MaxEntityCount && W->Actives[Entity], "DestroyEntity() error");
    W->Actives[Entity] = false;
    W->Components[Entity] = 0;
}
