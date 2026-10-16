/* A small Windows tile graphics library. Created by J. Nan, August 2008. */
#ifndef J_NAN_GAMECON_H_
#define J_NAN_GAMECON_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    E_SUCCESS = 0,
    E_NOT_INITIALIZED = -1,
    E_ALREADY_INITIALIZED = -2,
    E_PARAMETER = -3,
    E_INIT = -4
};

/* Limits apply independently to grid width/height in pixels and tile count. */
#define GC_MAX_PIXEL_DIMENSION 4096
#define GC_MAX_TILES 1048576u

/* Call all GC functions from the same application thread, including finalize.
 * The library runs its own UI thread; callers do not pump Windows messages.
 * title must be non-NULL; dimensions and blockSize must be positive and obey
 * the limits above. Initialization creates two black buffers. */
int GC_Init(const char *title, int width, int height, int blockSize);

/* Destroy the window, join its thread, and release resources. Also required
 * after the user closes the window. A later GC_Init may start a new session. */
int GC_Finalize(void);

/* Wait for a control-key transition or window closure, or up to ms milliseconds.
 * ms <= 0 means an indefinite wait, NOT a nonblocking poll. A positive result
 * is the remaining time after input interrupted the wait; zero means the wait
 * completed. Negative results are errors. Key events may coalesce. */
int GC_Sleep(int ms);

/* Set one back-buffer tile. Out-of-grid coordinates return E_PARAMETER;
 * color components use their low 8 bits. The back buffer is not cleared by Flip. */
int GC_Set(int x, int y, int red, int green, int blue);

/* Publish the back buffer for asynchronous painting. The previous front buffer
 * becomes the back buffer; publication does not wait for display presentation. */
int GC_Flip(void);

/* These functions return 1 while ANY mapped key is held, otherwise 0. They do
 * not consume press events. Focus loss clears held keys. Before initialization
 * and after window closure they return 0. Arrows/WASD map to directions;
 * J/K/U/I or numpad 1/2/4/5 map to A/B/C/D; Escape maps to Reset. */
int GC_UpPressed(void);
int GC_DownPressed(void);
int GC_LeftPressed(void);
int GC_RightPressed(void);
int GC_APressed(void);
int GC_BPressed(void);
int GC_CPressed(void);
int GC_DPressed(void);
int GC_ResetPressed(void);

/* Set, Flip, and Sleep return E_NOT_INITIALIZED if the window has closed.
 * Finalize still succeeds once to release that session's resources. */
#ifdef __cplusplus
}
#endif
#endif
