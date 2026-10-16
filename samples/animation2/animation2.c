#include "gamecon.h"
#include <stdio.h>

enum
{
    WIDTH = 100,
    HEIGHT = 40,
    BLOCK = 10,
    DELAY = 50,
    
    SKY_HEIGHT = 35,
    SUN_WIDTH = 9,
    SUN_HEIGHT = 9,
    SUN_X = 4,
    SUN_Y = 2,

    CLOUD_WIDTH = 19,
    CLOUD_HEIGHT = 10,
    CLOUD_Y = 5,
    CLOUD_COUNT = 2,
    CLOUD_SPACING = 35,
    CLOUD_SLOWNESS = 15,
    
    TREE_WIDTH = 11,
    TREE_HEIGHT = 15,
    TREE_Y = HEIGHT - TREE_HEIGHT,
    TREE_COUNT = 5,
    TREE_SPACING = 20,
    TREE_SLOWNESS = 1
};

void draw_background(void);
void draw_cloud(int x, int y);
void draw_tree(int x, int y);

int main(void)
{
    if (GC_Init("Game Console Sample: Animation 2", WIDTH, HEIGHT, BLOCK) != E_SUCCESS) {
        fprintf(stderr, "Unable to initialize GameCon.\n");
        return 1;
    }

    int cloud_x[CLOUD_COUNT];
    int tree_x[TREE_COUNT];

    // Initialize positions
    //
    int i;
    for(i = 0; i < CLOUD_COUNT; ++i)
        cloud_x[i] = (i + 1) * CLOUD_SPACING;
    for(i = 0; i < TREE_COUNT; ++i)
        tree_x[i] = (i + 1) * TREE_SPACING;

    // Animation
    //
    int step = 0;
    do
    {
        step = (step + 1) % (CLOUD_SLOWNESS * TREE_SLOWNESS);
        draw_background();

        int i;
        for(i = 0; i < CLOUD_COUNT; ++i)
        {
            draw_cloud(cloud_x[i], CLOUD_Y);

            if(step % CLOUD_SLOWNESS == 0)
            {
                cloud_x[i] = cloud_x[i] - 1;
                if(cloud_x[i] + CLOUD_WIDTH <= 0)
                    cloud_x[i] = WIDTH - 1;
            }
        }
        for(i = 0; i < TREE_COUNT; ++i)
        {
            draw_tree(tree_x[i], TREE_Y);

            if(step % TREE_SLOWNESS == 0)
            {
                tree_x[i] = tree_x[i] - 1;
                if(tree_x[i] + TREE_WIDTH <= 0)
                    tree_x[i] = WIDTH - 1;
            }
        }

        GC_Flip();
        if (GC_Sleep(DELAY) < 0) break;
    }
    while(!GC_ResetPressed());

    GC_Finalize();
    return 0;
}

void draw_background(void)
{
    static const char sun[SUN_HEIGHT][SUN_WIDTH] =
    {
        "    *    ",
        " *  *  * ",
        "  *****  ",
        "  *****  ",
        "*********",
        "  *****  ",
        "  *****  ",
        " *  *  * ",
        "    *    "
    };

    // Sky
    //
    int y;
    for(y = 0; y < SKY_HEIGHT; ++y)
    {
        int x;
        for(x = 0; x < WIDTH; ++x)
            GC_Set(x, y, 63, 72, 204);
    }

    // Earth
    //
    for(; y < HEIGHT; ++y)
    {
        int x;
        for(x = 0; x < WIDTH; ++x)
            GC_Set(x, y, 136, 0, 21);
    }

    // Sun
    //
    int i;
    for(i = 0; i < SUN_WIDTH; ++i)
    {
        int j;
        for(j = 0; j < SUN_HEIGHT; ++j)
        {
            if(sun[j][i] == '*')
                GC_Set(SUN_X + i, SUN_Y + j, 237, 28, 36);
        }
    }
}

void draw_cloud(int x, int y)
{
    static const char cloud[CLOUD_HEIGHT][CLOUD_WIDTH] =
    {
        "        *****      ",
        "   *** *********   ",
        " ***************** ",
        "*******************",
        "*******************",
        " ***************** ",
        "  *****************",
        "   ***** **********",
        "         ********* ",
        "           *****   "
    };

    int i;
    for(i = 0; i < CLOUD_WIDTH; ++i)
    {
        int j;
        for(j = 0; j < CLOUD_HEIGHT; ++j)
        {
            if(cloud[j][i] == '*')
                GC_Set(x + i, y + j, 255, 255, 255);
        }
    }
}

void draw_tree(int x, int y)
{
    static const char tree[TREE_HEIGHT][TREE_WIDTH] =
    {
        "     *     ",
        "    ***    ",
        "   *****   ",
        "  *******  ",
        "   *****   ",
        "  *******  ",
        " ********* ",
        "   *****   ",
        "  *******  ",
        " ********* ",
        "***********",
        "     *     ",
        "     *     ",
        "     *     ",
        "     *     "
    };

    int i;
    for(i = 0; i < TREE_WIDTH; ++i)
    {
        int j;
        for(j = 0; j < TREE_HEIGHT; ++j)
        {
            if(tree[j][i] == '*')
                GC_Set(x + i, y + j, 0, 200, 0);
        }
    }
}
