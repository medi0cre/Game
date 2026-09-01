#include <stdio.h>
#include <raylib.h>

int main(void)
{
    printf("Hello, World\n");
    printf("Directory Count: %d\n", GetDirectoryFileCount("./"));
    return 0;
}
