#include "gamecon.h"
#include <stdio.h>

enum
{
    WIDTH = 130,
    HEIGHT = 40,
    BLOCK = 10,
    FONT_WIDTH = 8,
    FONT_HEIGHT = 8,
    LINE_SPACING = 2

};

void clear(int r, int g, int b);
void draw_text(int x, int y, char text[], int r, int g, int b);
void draw_char(int x, int y, char c, int r, int g, int b);

int main(void)
{
    if (GC_Init("Game Console Sample: Text", WIDTH, HEIGHT, BLOCK) != E_SUCCESS) {
        fprintf(stderr, "Unable to initialize GameCon.\n");
        return 1;
    }

    clear(0, 0, 150);
    draw_text(4, 5, "The quick brown\nfox jumps over\nthe lazy dog", 255, 255, 0);
    GC_Flip();
    if (GC_Sleep(0) < 0) goto cleanup;
    if (GC_Sleep(0) < 0) goto cleanup;


    clear(0, 0, 0);
    draw_text(10, 10, "Game Over", 255, 0, 0);
    GC_Flip();
    if (GC_Sleep(0) < 0) goto cleanup;

cleanup:
    GC_Finalize();
    return 0;
}

void clear(int r, int g, int b)
{
    int x;
    for(x = 0; x < WIDTH; ++x)
    {
        int y;
        for(y = 0; y < HEIGHT; ++y)
            GC_Set(x, y, r, g, b);
    }
}

void draw_text(int x, int y, char text[], int r, int g, int b)
{
    int cx = x;
    int cy = y;

    int i;
    for(i = 0; text[i] != 0; ++i)
    {
        if(text[i] == '\n')
        {
            cy += (FONT_HEIGHT + LINE_SPACING);
            cx = x;
        }
        else
        {
            draw_char(cx, cy, text[i], r, g, b);
            cx += FONT_WIDTH;
        }
    }
}

void draw_char(int x, int y, char c, int r, int g, int b)
{
    static char font[27][FONT_HEIGHT][FONT_WIDTH] =
    {
        { "        ",
          "  ***   ",
          " ** **  ",
          "**   ** ",
          "**   ** ",
          "******* ",
          "**   ** ",
          "**   ** " },

        { "        ",
          "*****   ",
          "**  **  ",
          "**  **  ",
          "*****   ",
          "**  **  ",
          "**  **  ",
          "*****   " },

        { "        ",
          " ****   ",
          "**  **  ",
          "**      ",
          "**      ",
          "**      ",
          "**      ",
          " *****  " },

        { "        ",
          "*****   ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          "*****   " },

        { "        ",
          "******  ",
          "**      ",
          "**      ",
          "******  ",
          "**      ",
          "**      ",
          "******  " },

        { "        ",
          "******  ",
          "**      ",
          "**      ",
          "******  ",
          "**      ",
          "**      ",
          "**      " },

        { "        ",
          " ****   ",
          "**  **  ",
          "**      ",
          "**      ",
          "** ***  ",
          "**  **  ",
          " ****   " },

        { "        ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          "******* ",
          "**   ** ",
          "**   ** ",
          "**   ** " },

        { "        ",
          " ****   ",
          "  **    ",
          "  **    ",
          "  **    ",
          "  **    ",
          "  **    ",
          " ****   " },

        { "        ",
          "    **  ",
          "    **  ",
          "    **  ",
          "    **  ",
          "    **  ",
          "    **  ",
          "  ***   " },

        { "        ",
          "**   ** ",
          "**  **  ",
          "** **   ",
          "****    ",
          "** **   ",
          "**  **  ",
          "**   ** " },

        { "        ",
          " **     ",
          " **     ",
          " **     ",
          " **     ",
          " **     ",
          " **     ",
          " ****   " },

        { "        ",
          "**   ** ",
          "*** *** ",
          "******* ",
          "** * ** ",
          "**   ** ",
          "**   ** ",
          "**   ** " },

        { "        ",
          "**   ** ",
          "***  ** ",
          "**** ** ",
          "** **** ",
          "**  *** ",
          "**   ** ",
          "**   ** " },

        { "        ",
          " *****  ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          " *****  " },

        { "        ",
          "*****   ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          "*****   ",
          "**      ",
          "**      " },

        { "        ",
          " *****  ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          " *****  ",
          "    *** " },

        { "        ",
          "*****   ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          "*****   ",
          "** **   ",
          "**  **  " },

        { "        ",
          " ****   ",
          "**      ",
          " **     ",
          "  **    ",
          "   **   ",
          "   **   ",
          "****    " },

        { "        ",
          "******  ",
          "  **    ",
          "  **    ",
          "  **    ",
          "  **    ",
          "  **    ",
          "  **    " },

        { "        ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          " ****   " },

        { "        ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          " ** **  ",
          "  ***   " },

        { "        ",
          "**   ** ",
          "**   ** ",
          "**   ** ",
          "** * ** ",
          "******* ",
          "*** *** ",
          "**   ** " },

        { "        ",
          "**   ** ",
          "**   ** ",
          " ** **  ",
          "  ***   ",
          " ** **  ",
          "**   ** ",
          "**   ** " },

        { "        ",
          "**  **  ",
          "**  **  ",
          "**  **  ",
          " ****   ",
          "  **    ",
          "  **    ",
          "  **    " },

        { "        ",
          "******  ",
          "    **  ",
          "   **   ",
          "  **    ",
          " **     ",
          "**      ",
          "******  " },

        { "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        ",
          "        " }
    };

    int iChar;
    if(c >= 'a' && c <= 'z')
        iChar = c - 'a';
    else if(c >= 'A' && c <= 'Z')
        iChar = c - 'A';
    else
        iChar = 26;

    int i;
    for(i = 0; i < FONT_WIDTH; ++i)
    {
        int j;
        for(j = 0; j < FONT_HEIGHT; ++j)
        {
            if(font[iChar][j][i] == '*')
                GC_Set(x + i, y + j, r, g, b);
        }
    }
}
