#include "gamecon.h"
#include <stdio.h>

enum
{
    WIDTH = 20,
    HEIGHT = 20,
    BLOCK = 20
};

int main(void)
{
    if (GC_Init("Game Console Test", WIDTH, HEIGHT, BLOCK) != E_SUCCESS) {
        fprintf(stderr, "Unable to initialize GameCon.\n");
        return 1;
    }

    int px = 0;
    int py = 0;
    int stop = 0;
    while(!stop)
    {
	if(GC_LeftPressed())
	{
	    px--;
	    if(px < 0)
		px = WIDTH - 1;
	}
	else if(GC_RightPressed())
	{
	    px++;
	    if(px == WIDTH)
		px = 0;
	}
        else if(GC_UpPressed())
	{
	    py--;
	    if(py < 0)
		py = HEIGHT - 1;
	}
	else if(GC_DownPressed())
	{
	    py++;
	    if(py == HEIGHT)
		py = 0;
	}

	if(GC_ResetPressed())
	    stop = 1;

	int x;
	for(x = 0; x < WIDTH; x++)
	{
	    int y;
	    for(y = 0; y < HEIGHT; y++)
	    {
		if(x == px && y == py)
		    GC_Set(x, y, 255, 0, 0);
		else
		    GC_Set(x, y, 0, 100, 0);
	    }
	}

	GC_Flip();
	if (!stop && GC_Sleep(0) < 0) break;
    }

    GC_Finalize();
    return 0;
}
