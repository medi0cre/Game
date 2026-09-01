#include <raylib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <Utils.h>

void Enforce(bool Condition, const char* Message)
{
    if (Condition) { return; }
    TraceLog(LOG_FATAL, Message);
    exit(EXIT_FAILURE);
}
