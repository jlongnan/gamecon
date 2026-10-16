#include "gamecon.h"
#include <stdio.h>
#include <stdlib.h>

enum
{
    WIDTH = 80,
    HEIGHT = 70,
    BLOCK = 7,
    DELAY = 50,
    MAX_BOX_COUNT = 15,
    MIN_VELOCITY = 50,
    MAX_VELOCITY = 255,
    SMALL_BOX_SIZE = 2,
    BOX_SPACING = 5,

    I_X = 0,
    I_Y = 1,
    I_S = 2,
    I_VX = 3,
    I_VY = 4
};

int make_box(int boxes[MAX_BOX_COUNT][5], int count, int size);
int check_box(int boxes[MAX_BOX_COUNT][5], int count, int i);
int check_overlap(int box1[5], int box2[5]);
void move(int boxes[MAX_BOX_COUNT][5], int count, int step);
void draw_background(void);
void draw_box(int box[5]);

int main(void)
{
    if (GC_Init("Game Console Sample: Animation 3", WIDTH, HEIGHT, BLOCK) != E_SUCCESS) {
        fprintf(stderr, "Unable to initialize GameCon.\n");
        return 1;
    }

    int boxes[MAX_BOX_COUNT][5];
    int count = 0;
    int size = SMALL_BOX_SIZE;

    // Animation
    //
    int step = 0;
    do
    {
        // Make
        //
        if(count < MAX_BOX_COUNT && step % 60 == 0)
        {
            if(make_box(boxes, count, size))
            {
                ++count;
                ++size;
            }
        }
        ++step;

        // Draw
        //
        draw_background();
        int i;
        for(i = 0; i < count; ++i)
            draw_box(boxes[i]);

        // Move
        //
        move(boxes, count, step);

        GC_Flip();
        if (GC_Sleep(DELAY) < 0) break;
    }
    while(!GC_ResetPressed());

    GC_Finalize();
    return 0;
}

int make_box(int boxes[MAX_BOX_COUNT][5], int count, int size)
{
    boxes[count][I_S] = size;

    int maxpos;
    if(count % 2 == 0)
        maxpos = WIDTH;
    else
        maxpos = HEIGHT;

    int pos;
    for(pos = BOX_SPACING; pos < maxpos; pos += BOX_SPACING)
    {
        if(count % 2 == 0)
        {
            boxes[count][I_X] = 0;
            boxes[count][I_Y] = pos;
            boxes[count][I_VX] = MIN_VELOCITY;
            boxes[count][I_VY] = 0;
        }
        else
        {
            boxes[count][I_X] = pos;
            boxes[count][I_Y] = 0;
            boxes[count][I_VX] = 0;
            boxes[count][I_VY] = MIN_VELOCITY;
        }

        if(check_box(boxes, count, count))
            return 1;
    }

    return 0;
}

int check_box(int boxes[MAX_BOX_COUNT][5], int count, int i)
{
    // Out of boundary?
    //
    if(boxes[i][I_X] < 0 || boxes[i][I_X] + boxes[i][I_S] > WIDTH ||
            boxes[i][I_Y] < 0 || boxes[i][I_Y] + boxes[i][I_S] > HEIGHT)
    {
        return 0;
    }

    // Overlapping?
    //
    int j;
    for(j = 0; j < count; ++j)
    {
        if(j != i && check_overlap(boxes[i], boxes[j]))
            return 0;
    }

    // OK
    //
    return 1;
}

int check_overlap(int box1[5], int box2[5])
{
    if(box1[I_X] >= box2[I_X] + box2[I_S] ||
            box2[I_X] >= box1[I_X] + box1[I_S] ||
            box1[I_Y] >= box2[I_Y] + box2[I_S] ||
            box2[I_Y] >= box1[I_Y] + box1[I_S])
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void move(int boxes[MAX_BOX_COUNT][5], int count, int step)
{
    int i;
    for(i = 0; i < count; ++i)
    {
        if(step % (MAX_VELOCITY / abs(boxes[i][I_VX] + boxes[i][I_VY])) == 0)
        {
            int ox = boxes[i][I_X];
            int oy = boxes[i][I_Y];
            if(boxes[i][I_VX] != 0)
                boxes[i][I_X] += boxes[i][I_VX] / abs(boxes[i][I_VX]);
            else
                boxes[i][I_Y] += boxes[i][I_VY] / abs(boxes[i][I_VY]);

            if(check_box(boxes, count, i))
            {
                if(boxes[i][I_VX] > 0)
                {
                    if(boxes[i][I_VX] < MAX_VELOCITY)
                        ++boxes[i][I_VX];
                }
                else if(boxes[i][I_VX] < 0)
                {
                    if(boxes[i][I_VX] > -MAX_VELOCITY)
                        --boxes[i][I_VX];
                }
                else if(boxes[i][I_VY] > 0)
                {
                    if(boxes[i][I_VY] < MAX_VELOCITY)
                        ++boxes[i][I_VY];
                }
                else
                {
                    if(boxes[i][I_VY] > -MAX_VELOCITY)
                        --boxes[i][I_VY];
                }
            }
            else
            {
                boxes[i][I_X] = ox;
                boxes[i][I_Y] = oy;
                boxes[i][I_VX] = -boxes[i][I_VX];
                boxes[i][I_VY] = -boxes[i][I_VY];
            }
        }
    }
}

void draw_background(void)
{
    // Black background
    //
    int y;
    for(y = 0; y < HEIGHT; ++y)
    {
        int x;
        for(x = 0; x < WIDTH; ++x)
            GC_Set(x, y, 0, 0, 0);
    }
}

void draw_box(int box[5])
{
    int v;
    if(box[I_VX] != 0)
        v = abs(box[I_VX]);
    else
        v = abs(box[I_VY]);
    int gb = 255 * (MAX_VELOCITY - v) / (MAX_VELOCITY - MIN_VELOCITY);

    int y;
    for(y = 0; y < box[I_S]; ++y)
    {
        int x;
        for(x = 0; x < box[I_S]; ++x)
            GC_Set(box[I_X] + x, box[I_Y] + y, 255, gb, gb);
    }
}
