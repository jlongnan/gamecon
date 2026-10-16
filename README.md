# gamecon

A small C library for writing tile-based GUI games on Windows, intended for
students learning C. Game code draws colored tiles and reads controls through
`gamecon.h`; the library owns the Windows message loop.

## Build on Windows

Install [MSYS2](https://www.msys2.org/) and open its **UCRT64** terminal. Install
the compiler and GNU Make:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc make git
git clone https://github.com/jlongnan/gamecon.git
cd gamecon
make
make test
./samples/animation2/animation2.exe
```

`make` builds `libgamecon.a` and all six examples. `make test` builds and runs the
Windows regression suite and checks that every example opens and exits on window
closure. `make clean` removes build products. To build just the library, use `make libgamecon.a`; a sample can also be built from its own
directory with `make`. Library and header changes rebuild dependent examples.

GNU Make and MinGW-w64 must be available in your shell. The library targets the
Windows API; Linux's native GCC is not sufficient. Linux cross compilation is
supported with explicit tool overrides:

```sh
make CC=x86_64-w64-mingw32-gcc AR=x86_64-w64-mingw32-ar
make test CC=x86_64-w64-mingw32-gcc RUNNER=wine
```

Wine tests require a working graphical display (or Xvfb). Windows CI builds all
examples with warnings treated as errors and runs the same regression suite.
Link your own program with `-L/path/to/gamecon -lgamecon -lgdi32` and add
`-I/path/to/gamecon` when compiling. Add `-mwindows` for a GUI-subsystem program.

## API and lifecycle

Call all `GC_*` functions from **one application thread**. The library creates
and synchronizes its own UI thread. Concurrent public calls from additional
application threads are not supported. Only one GameCon window can be active.

- `GC_Init(title, width, height, blockSize)` creates two black tile buffers and a
  window. Check its return value before entering a game loop. The title must be
  non-NULL; sizes must be positive. Each grid dimension multiplied by block size
  must be at most `GC_MAX_PIXEL_DIMENSION` (4096 pixels), and the grid must have
  at most `GC_MAX_TILES` (1,048,576 tiles). These limits bound memory and drawing
  costs; allocation or window creation can still fail with `E_INIT`.
- `GC_Set(x, y, red, green, blue)` changes a back-buffer tile. Coordinates start
  at the top left. Out-of-range coordinates return `E_PARAMETER` and draw
  nothing, allowing sprites to be clipped at grid edges. Color components use
  their low eight bits; use values from 0 through 255.
- `GC_Flip()` publishes the back buffer for asynchronous painting and makes the
  previous front buffer the back buffer. It does **not** clear that buffer or
  wait for the screen to display the frame. Redraw a complete frame when needed.
- `GC_Sleep(ms)` waits for a control-key transition, window closure, or a timeout.
  **Zero and negative values wait indefinitely.** A positive return value is
  the time remaining when input interrupted a timed wait; zero means the wait
  completed. Input can wake a wait early, and multiple transitions may coalesce;
  this is not an event queue or a guaranteed frame-rate limiter.
- `GC_*Pressed()` returns whether a mapped key is currently held, not whether a
  new press occurred. Either alias keeps an action held; losing focus clears
  all held keys. These functions return zero outside an open session.
- `GC_Finalize()` destroys the window on its owning thread, waits for that thread
  to exit, and releases resources. Call it even after the user closes the
  window. A later `GC_Init()` can create another session.

`GC_Init`, `GC_Set`, `GC_Flip`, and `GC_Finalize` return `E_SUCCESS` (0) on success.
Negative values indicate errors: `E_NOT_INITIALIZED`, `E_ALREADY_INITIALIZED`,
`E_PARAMETER`, or `E_INIT`. After window closure, `GC_Set`, `GC_Flip`, and
`GC_Sleep` return `E_NOT_INITIALIZED`; leave the game loop and finalize.

```c
#include <stdio.h>
#include "gamecon.h"

int main(void)
{
    if (GC_Init("My game", 20, 20, 20) != E_SUCCESS) {
        fprintf(stderr, "Unable to initialize GameCon.\n");
        return 1;
    }
    while (!GC_ResetPressed()) {
        for (int y = 0; y < 20; ++y)
            for (int x = 0; x < 20; ++x)
                GC_Set(x, y, 30, 60, 100);
        if (GC_Flip() < 0 || GC_Sleep(50) < 0)
            break;
    }
    return GC_Finalize() == E_SUCCESS ? 0 : 1;
}
```

## Controls and examples

| Action | Keys |
| --- | --- |
| Up / Down / Left / Right | Arrow keys or W / S / A / D |
| A / B / C / D | J / K / U / I or numpad 1 / 2 / 4 / 5 |
| Reset | Escape |

`display` shows tile colors, `animation` moves a tile, `animation2` animates a
landscape, `animation3` moves colliding boxes, `control` responds to directional
input, and `text` draws bitmap letters. The animation2, animation3, and control
examples exit on Escape. Other examples advance or finish on control-key
transitions. All examples exit when their window is closed.

## Validation

`tests/regression.c` covers invalid and oversized dimensions, duplicate
initialization, alias keys and focus loss, rendered pixels after frame publication,
repainting during frame updates and shutdown, window closure interrupting an
indefinite wait, and repeated initialization/finalization without leaking thread
handles. The test requires a graphical Windows session or Wine with a display.
A passing stress test supplements the locking design; it cannot prove the
absence of every possible thread interleaving.

## Screenshots

![animation2](samples/animation2/screenshot.png)
![text](samples/text/screenshot.png)
