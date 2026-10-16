/*
 * gamecon.c
 * A C library that allows students to write simple Windows GUI games
 * Created by J. Nan in August, 2008
 */
#include "gamecon.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>

static const char gc_className[] = "GameCon20090828";
enum { LEFT = 20, TOP = 20, PANEL = 150 };

/* Public calls belong to one application thread. The critical section protects
 * frame publication and key state shared with the internal window thread. */
static int gc_initialized;
static int gc_ready;
static HINSTANCE gc_instance;
static HWND gc_hwnd;
static HANDLE gc_thread;
static DWORD gc_threadId;
static HANDLE gc_buttonEvent;
static CRITICAL_SECTION gc_lock;
static LONG gc_closed;
static int gc_width, gc_height, gc_blockSize;
static COLORREF *gc_frontBuffer, *gc_backBuffer;
static unsigned char gc_keys[256];

struct Startup {
    const char *title;
    HANDLE ready;
};

static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
static DWORD WINAPI UI(LPVOID);

static int IsOpen(void)
{
    return gc_initialized && InterlockedCompareExchange(&gc_closed, 0, 0) == 0;
}

static void ReleaseResources(void)
{
    if (gc_thread) CloseHandle(gc_thread);
    if (gc_buttonEvent) CloseHandle(gc_buttonEvent);
    free(gc_frontBuffer);
    free(gc_backBuffer);
    gc_thread = NULL;
    gc_buttonEvent = NULL;
    gc_frontBuffer = gc_backBuffer = NULL;
    gc_hwnd = NULL;
    gc_initialized = 0;
    DeleteCriticalSection(&gc_lock);
}

int GC_Init(const char *title, int width, int height, int blockSize)
{
    size_t cells, bytes;
    HANDLE waits[2];
    struct Startup startup;
    DWORD result;

    if (gc_initialized) return E_ALREADY_INITIALIZED;
    if (!title || width <= 0 || height <= 0 || blockSize <= 0 ||
        blockSize > GC_MAX_PIXEL_DIMENSION ||
        width > GC_MAX_PIXEL_DIMENSION / blockSize ||
        height > GC_MAX_PIXEL_DIMENSION / blockSize)
        return E_PARAMETER;
    if ((size_t)width > (size_t)-1 / (size_t)height)
        return E_PARAMETER;
    cells = (size_t)width * (size_t)height;
    if (cells > GC_MAX_TILES || cells > (size_t)-1 / sizeof(COLORREF))
        return E_PARAMETER;
    bytes = cells * sizeof(COLORREF);

    if (!gc_ready) {
        WNDCLASSEXA wc;
        memset(&wc, 0, sizeof(wc));
        gc_instance = GetModuleHandleA(NULL);
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = gc_instance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = gc_className;
        if (!RegisterClassExA(&wc)) return E_INIT;
        gc_ready = 1;
    }

    InitializeCriticalSection(&gc_lock);
    gc_width = width;
    gc_height = height;
    gc_blockSize = blockSize;
    gc_hwnd = NULL;
    memset(gc_keys, 0, sizeof(gc_keys));
    InterlockedExchange(&gc_closed, 0);
    gc_frontBuffer = (COLORREF *)calloc(1, bytes);
    gc_backBuffer = (COLORREF *)calloc(1, bytes);
    gc_buttonEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    startup.title = title;
    startup.ready = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!gc_frontBuffer || !gc_backBuffer || !gc_buttonEvent || !startup.ready) {
        if (startup.ready) CloseHandle(startup.ready);
        ReleaseResources();
        return E_INIT;
    }

    gc_thread = CreateThread(NULL, 0, UI, &startup, 0, &gc_threadId);
    if (!gc_thread) {
        CloseHandle(startup.ready);
        ReleaseResources();
        return E_INIT;
    }
    waits[0] = startup.ready;
    waits[1] = gc_thread;
    result = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
    /* The UI has copied startup before signalling ready. Join on failure before
     * the stack arguments, event, or buffers can go away. */
    if (result != WAIT_OBJECT_0 || !gc_hwnd) {
        PostThreadMessage(gc_threadId, WM_QUIT, 0, 0);
        WaitForSingleObject(gc_thread, INFINITE);
        CloseHandle(startup.ready);
        ReleaseResources();
        return E_INIT;
    }
    CloseHandle(startup.ready);
    gc_initialized = 1;
    return E_SUCCESS;
}

int GC_Finalize(void)
{
    if (!gc_initialized) return E_NOT_INITIALIZED;
    if (WaitForSingleObject(gc_thread, 0) == WAIT_TIMEOUT) {
        /* Wake the message loop; UI destroys its own window before exiting. */
        if (!PostThreadMessage(gc_threadId, WM_QUIT, 0, 0) &&
            WaitForSingleObject(gc_thread, 0) != WAIT_OBJECT_0)
            return E_INIT;
    }
    if (WaitForSingleObject(gc_thread, INFINITE) != WAIT_OBJECT_0)
        return E_INIT;
    ReleaseResources();
    return E_SUCCESS;
}

int GC_Sleep(int ms)
{
    DWORD start, result, elapsed;
    if (!IsOpen()) return E_NOT_INITIALIZED;
    start = GetTickCount();
    result = WaitForSingleObject(gc_buttonEvent, ms > 0 ? (DWORD)ms : INFINITE);
    if (!IsOpen()) return E_NOT_INITIALIZED;
    if (result == WAIT_FAILED) return E_INIT;
    elapsed = GetTickCount() - start;
    if (result == WAIT_TIMEOUT || ms <= 0 || elapsed >= (DWORD)ms) return 0;
    return ms - (int)elapsed;
}

int GC_Set(int x, int y, int red, int green, int blue)
{
    if (!IsOpen()) return E_NOT_INITIALIZED;
    if (x < 0 || x >= gc_width || y < 0 || y >= gc_height)
        return E_PARAMETER;
    /* Only the application thread writes the back buffer. Flip waits for any
     * paint to finish before making the previous front buffer writable. */
    gc_backBuffer[(size_t)y * (size_t)gc_width + (size_t)x] = RGB(red, green, blue);
    return E_SUCCESS;
}

int GC_Flip(void)
{
    COLORREF *temp;
    RECT rect;
    if (!IsOpen()) return E_NOT_INITIALIZED;
    EnterCriticalSection(&gc_lock);
    temp = gc_frontBuffer;
    gc_frontBuffer = gc_backBuffer;
    gc_backBuffer = temp;
    rect.left = LEFT;
    rect.top = TOP;
    rect.right = LEFT + gc_width * gc_blockSize;
    rect.bottom = TOP + gc_height * gc_blockSize;
    InvalidateRect(gc_hwnd, &rect, FALSE);
    LeaveCriticalSection(&gc_lock);
    return E_SUCCESS;
}

static int KeyHeld(int first, int second)
{
    int held;
    if (!IsOpen()) return 0;
    EnterCriticalSection(&gc_lock);
    held = gc_keys[first] || gc_keys[second];
    LeaveCriticalSection(&gc_lock);
    return held;
}

int GC_UpPressed(void)    { return KeyHeld(VK_UP, 'W'); }
int GC_DownPressed(void)  { return KeyHeld(VK_DOWN, 'S'); }
int GC_LeftPressed(void)  { return KeyHeld(VK_LEFT, 'A'); }
int GC_RightPressed(void) { return KeyHeld(VK_RIGHT, 'D'); }
int GC_APressed(void)     { return KeyHeld(VK_NUMPAD1, 'J'); }
int GC_BPressed(void)     { return KeyHeld(VK_NUMPAD2, 'K'); }
int GC_CPressed(void)     { return KeyHeld(VK_NUMPAD4, 'U'); }
int GC_DPressed(void)     { return KeyHeld(VK_NUMPAD5, 'I'); }
int GC_ResetPressed(void) { return KeyHeld(VK_ESCAPE, VK_ESCAPE); }

static int IsControlKey(WPARAM key)
{
    switch (key) {
    case VK_UP: case 'W': case VK_DOWN: case 'S':
    case VK_LEFT: case 'A': case VK_RIGHT: case 'D':
    case VK_NUMPAD1: case 'J': case VK_NUMPAD2: case 'K':
    case VK_NUMPAD4: case 'U': case VK_NUMPAD5: case 'I':
    case VK_ESCAPE: return 1;
    default: return 0;
    }
}

static void ClearKeys(void)
{
    size_t i;
    int changed = 0;
    EnterCriticalSection(&gc_lock);
    for (i = 0; i < sizeof(gc_keys); ++i) {
        changed |= gc_keys[i];
        gc_keys[i] = 0;
    }
    LeaveCriticalSection(&gc_lock);
    if (changed) SetEvent(gc_buttonEvent);
}

static void Paint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hwnd, &ps);
    int width = gc_width * gc_blockSize;
    int height = gc_height * gc_blockSize;
    HDC back = CreateCompatibleDC(dc);
    HBITMAP bitmap = CreateCompatibleBitmap(dc, width, height);
    if (back && bitmap) {
        HGDIOBJ oldBitmap = SelectObject(back, bitmap);
        HGDIOBJ oldBrush = SelectObject(back, GetStockObject(DC_BRUSH));
        int x, y;
        EnterCriticalSection(&gc_lock);
        for (y = 0; y < gc_height; ++y) {
            for (x = 0; x < gc_width; ++x) {
                int px = x * gc_blockSize, py = y * gc_blockSize;
                SetDCBrushColor(back, gc_frontBuffer[(size_t)y * (size_t)gc_width + (size_t)x]);
                Rectangle(back, px, py, px + gc_blockSize, py + gc_blockSize);
            }
        }
        LeaveCriticalSection(&gc_lock);
        BitBlt(dc, LEFT, TOP, width, height, back, 0, 0, SRCCOPY);
        SelectObject(back, oldBrush);
        SelectObject(back, oldBitmap);
    }
    if (bitmap) DeleteObject(bitmap);
    if (back) DeleteDC(back);
    EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_PAINT:
        Paint(hwnd);
        return 0;
    case WM_KEYDOWN:
    case WM_KEYUP:
        if (IsControlKey(wParam)) {
            unsigned char down = (message == WM_KEYDOWN);
            int changed;
            EnterCriticalSection(&gc_lock);
            changed = gc_keys[wParam] != down;
            gc_keys[wParam] = down;
            LeaveCriticalSection(&gc_lock);
            if (changed) SetEvent(gc_buttonEvent);
            return 0;
        }
        break;
    case WM_KILLFOCUS:
        ClearKeys();
        return 0;
    case WM_DESTROY:
        InterlockedExchange(&gc_closed, 1);
        ClearKeys();
        SetEvent(gc_buttonEvent);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, message, wParam, lParam);
}

static DWORD WINAPI UI(LPVOID argument)
{
    struct Startup startup = *(struct Startup *)argument;
    MSG message;
    int result;
    HWND hwnd = CreateWindowA(gc_className, startup.title, WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT,
        LEFT * 2 + 5 + gc_width * gc_blockSize,
        TOP + PANEL + gc_height * gc_blockSize, NULL, NULL, gc_instance, NULL);
    gc_hwnd = hwnd;
    if (!hwnd) {
        SetEvent(startup.ready);
        return 1;
    }
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    SetEvent(startup.ready);
    while ((result = GetMessageA(&message, NULL, 0, 0)) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    if (IsWindow(hwnd)) DestroyWindow(hwnd);
    InterlockedExchange(&gc_closed, 1);
    SetEvent(gc_buttonEvent);
    return result < 0 ? 1u : 0u;
}
