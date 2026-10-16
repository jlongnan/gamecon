#include "gamecon.h"
#include <stdio.h>

enum
{
    WIDTH = 20,
    HEIGHT = 20,
    BLOCK = 20
};

int draw(int x, int y);

int main(void)
{
    if (GC_Init("Game Console Sample: Animation", WIDTH, HEIGHT, BLOCK) != E_SUCCESS) {
        fprintf(stderr, "Unable to initialize GameCon.\n");
        return 1;
    }

    int x;
    for(x = 0; x < WIDTH; ++x)
        if (draw(x, 0) < 0) goto cleanup;

    int y;
    for(y = 1; y < HEIGHT; ++y)
        if (draw(WIDTH - 1, y) < 0) goto cleanup;

    for(x = WIDTH - 2; x >= 0; --x)
        if (draw(x, HEIGHT - 1) < 0) goto cleanup;

    for(y = HEIGHT - 2; y >= 0; --y)
        if (draw(0, y) < 0) goto cleanup;

    GC_Sleep(0);
cleanup:
    GC_Finalize();
    return 0;
}

int draw(int x, int y)
{
    int i;
    for(i = 0; i < WIDTH; ++i)
    {
        int j;
        for(j = 0; j < HEIGHT; ++j)
            GC_Set(i, j, 0, 0, 100);
    }

    GC_Set(x, y, 255, 255, 0);
    GC_Flip();
    return GC_Sleep(100);
}
