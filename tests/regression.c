#include <windows.h>
#include <limits.h>
#include <stdio.h>
#include "gamecon.h"

static int checks;
#define CHECK(expression) do { \
    ++checks; \
    if (!(expression)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expression); \
        ExitProcess(1); \
    } \
} while (0)

static HWND Window(void)
{
    HWND hwnd = FindWindowA("GameCon20090828", "GameCon regression");
    CHECK(hwnd != NULL);
    return hwnd;
}

static void PaintNow(HWND hwnd)
{
    DWORD_PTR result;
    CHECK(SendMessageTimeout(hwnd, WM_PAINT, 0, 0, SMTO_ABORTIFHUNG,
                             5000, &result) != 0);
}

static void TestParameters(void)
{
    CHECK(GC_Set(0, 0, 0, 0, 0) == E_NOT_INITIALIZED);
    CHECK(GC_Flip() == E_NOT_INITIALIZED);
    CHECK(GC_Sleep(1) == E_NOT_INITIALIZED);
    CHECK(GC_Finalize() == E_NOT_INITIALIZED);
    CHECK(GC_UpPressed() == 0);
    CHECK(GC_Init(NULL, 4, 4, 10) == E_PARAMETER);
    CHECK(GC_Init("invalid", 0, 4, 10) == E_PARAMETER);
    CHECK(GC_Init("invalid", 4, -1, 10) == E_PARAMETER);
    CHECK(GC_Init("invalid", 4, 4, 0) == E_PARAMETER);
    CHECK(GC_Init("overflow", INT_MAX, INT_MAX, INT_MAX) == E_PARAMETER);
    CHECK(GC_Init("overflow", 65536, 16384, 1) == E_PARAMETER);
    CHECK(GC_Init("too many tiles", 2048, 2048, 1) == E_PARAMETER);
    CHECK(GC_Init("too many pixels", 4, 4, GC_MAX_PIXEL_DIMENSION) == E_PARAMETER);
}

static void TestKeys(HWND hwnd)
{
    SendMessage(hwnd, WM_KEYDOWN, 'W', 0);
    CHECK(GC_UpPressed() == 1);
    SendMessage(hwnd, WM_KEYDOWN, VK_UP, 0);
    SendMessage(hwnd, WM_KEYUP, 'W', 0);
    CHECK(GC_UpPressed() == 1);
    SendMessage(hwnd, WM_KEYUP, VK_UP, 0);
    CHECK(GC_UpPressed() == 0);
    SendMessage(hwnd, WM_KEYDOWN, 'J', 0);
    SendMessage(hwnd, WM_KEYDOWN, VK_NUMPAD1, 0);
    SendMessage(hwnd, WM_KEYUP, 'J', 0);
    CHECK(GC_APressed() == 1);
    SendMessage(hwnd, WM_KEYDOWN, VK_RIGHT, 0);
    SendMessage(hwnd, WM_KILLFOCUS, 0, 0);
    CHECK(GC_APressed() == 0);
    CHECK(GC_RightPressed() == 0);
    /* Consume the coalesced input wakeup, then verify a real timeout. */
    CHECK(GC_Sleep(1) >= 0);
    {
        DWORD start = GetTickCount();
        CHECK(GC_Sleep(30) == 0);
        CHECK(GetTickCount() - start >= 20);
    }
}

static void TestFrames(HWND hwnd)
{
    int frame, x, y;
    for (frame = 0; frame < 30; ++frame) {
        int red = frame % 2 ? 255 : 0;
        HDC dc;
        for (y = 0; y < 8; ++y)
            for (x = 0; x < 8; ++x)
                CHECK(GC_Set(x, y, red, 40, 80) == E_SUCCESS);
        CHECK(GC_Flip() == E_SUCCESS);
        PaintNow(hwnd);
        dc = GetDC(hwnd);
        CHECK(dc != NULL);
        CHECK(GetPixel(dc, 25, 25) == RGB(red, 40, 80));
        CHECK(GetPixel(dc, 95, 95) == RGB(red, 40, 80));
        ReleaseDC(hwnd, dc);
    }
    CHECK(GC_Set(-1, 0, 0, 0, 0) == E_PARAMETER);
    CHECK(GC_Set(8, 0, 0, 0, 0) == E_PARAMETER);
    CHECK(GC_Set(0, 8, 0, 0, 0) == E_PARAMETER);
}

/* Hammer synchronous repaint requests while the application writes and flips.
 * The worker belongs to the test, not the library's public calling contract. */
struct Painter { HWND hwnd; LONG stop; };
static DWORD WINAPI Repaint(LPVOID argument)
{
    struct Painter *painter = (struct Painter *)argument;
    while (!InterlockedCompareExchange(&painter->stop, 0, 0)) {
        DWORD_PTR result;
        SendMessageTimeout(painter->hwnd, WM_PAINT, 0, 0,
                           SMTO_ABORTIFHUNG, 1000, &result);
    }
    return 0;
}

static void TestRepaintAndShutdown(HWND hwnd)
{
    struct Painter painter = { hwnd, 0 };
    HANDLE worker = CreateThread(NULL, 0, Repaint, &painter, 0, NULL);
    HANDLE ui = OpenThread(SYNCHRONIZE, FALSE, GetWindowThreadProcessId(hwnd, NULL));
    int frame, x, y;
    CHECK(worker != NULL && ui != NULL);
    for (frame = 0; frame < 100; ++frame) {
        for (y = 0; y < 8; ++y)
            for (x = 0; x < 8; ++x)
                CHECK(GC_Set(x, y, frame, 0, 0) == E_SUCCESS);
        CHECK(GC_Flip() == E_SUCCESS);
    }
    /* Keep repaint requests active while shutdown destroys the window. */
    CHECK(GC_Finalize() == E_SUCCESS);
    CHECK(!IsWindow(hwnd));
    CHECK(WaitForSingleObject(ui, 1000) == WAIT_OBJECT_0);
    InterlockedExchange(&painter.stop, 1);
    CHECK(WaitForSingleObject(worker, 5000) == WAIT_OBJECT_0);
    CloseHandle(ui);
    CloseHandle(worker);
}

static DWORD WINAPI CloseSoon(LPVOID argument)
{
    Sleep(50);
    PostMessage((HWND)argument, WM_CLOSE, 0, 0);
    return 0;
}

static void TestCloseWakesSleep(void)
{
    HANDLE closer;
    HWND hwnd;
    DWORD start;
    CHECK(GC_Init("GameCon regression", 8, 8, 10) == E_SUCCESS);
    hwnd = Window();
    closer = CreateThread(NULL, 0, CloseSoon, hwnd, 0, NULL);
    CHECK(closer != NULL);
    start = GetTickCount();
    CHECK(GC_Sleep(0) == E_NOT_INITIALIZED);
    CHECK(GetTickCount() - start < 5000);
    CHECK(WaitForSingleObject(closer, 1000) == WAIT_OBJECT_0);
    CloseHandle(closer);
    CHECK(GC_UpPressed() == 0);
    CHECK(GC_Set(0, 0, 0, 0, 0) == E_NOT_INITIALIZED);
    CHECK(GC_Flip() == E_NOT_INITIALIZED);
    CHECK(GC_Finalize() == E_SUCCESS);
    CHECK(!IsWindow(hwnd));
}

int main(void)
{
    int iteration;
    DWORD baseline, after;
    HWND hwnd;
    TestParameters();
    CHECK(GC_Init("GameCon regression", 8, 8, 10) == E_SUCCESS);
    CHECK(GC_Init("duplicate", 8, 8, 10) == E_ALREADY_INITIALIZED);
    hwnd = Window();
    PaintNow(hwnd);
    TestKeys(hwnd);
    TestFrames(hwnd);
    TestRepaintAndShutdown(hwnd);
    TestCloseWakesSleep();
    CHECK(GetProcessHandleCount(GetCurrentProcess(), &baseline));
    for (iteration = 0; iteration < 12; ++iteration) {
        CHECK(GC_Init("GameCon regression", 8, 8, 10) == E_SUCCESS);
        CHECK(GC_Finalize() == E_SUCCESS);
        CHECK(FindWindowA("GameCon20090828", "GameCon regression") == NULL);
    }
    CHECK(GetProcessHandleCount(GetCurrentProcess(), &after));
    CHECK(after <= baseline + 2);
    CHECK(GC_Finalize() == E_NOT_INITIALIZED);
    printf("PASS: %d checks (parameters, keys, frames, repaint/shutdown, close/wait, repeated lifecycle)\n", checks);
    return 0;
}
