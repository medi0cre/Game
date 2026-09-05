#include <stdio.h>
#include <Entity.h>
#include <Utils.h>

Entity* Entities = NULL;

uint16_t CreateEntity(void)
{
    for (uint16_t i = 0; i < EntityMax; i++)
    {
        if (!Entities[i].Active)
        {
            Entities[i].Active = true;
            Entities[i].Components = 0;
            return i;
        }
    }

    return UINT16_MAX;
}

void DestroyEntity(uint16_t _Entity_)
{
    Enforce(_Entity_ < EntityMax, "Entity index exceeded max limit");
    Entities[_Entity_].Active = false;
    Entities[_Entity_].Components = 0;
}
