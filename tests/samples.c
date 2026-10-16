/* Run each real example and close it through the normal Windows message path. */
#include <windows.h>
#include <stdio.h>

struct Search { DWORD pid; HWND window; };

static BOOL CALLBACK FindSample(HWND hwnd, LPARAM argument)
{
    struct Search *search = (struct Search *)argument;
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == search->pid && IsWindowVisible(hwnd)) {
        search->window = hwnd;
        return FALSE;
    }
    return TRUE;
}

int main(void)
{
    static const char *names[] = {
        "animation", "animation2", "animation3", "control", "display", "text"
    };
    size_t i;
    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        char path[128];
        STARTUPINFOA startup;
        PROCESS_INFORMATION process;
        struct Search search;
        DWORD exitCode;
        int attempt;
        ZeroMemory(&startup, sizeof(startup));
        startup.cb = sizeof(startup);
        snprintf(path, sizeof(path), "samples\\%s\\%s.exe", names[i], names[i]);
        if (!CreateProcessA(path, NULL, NULL, NULL, FALSE, 0, NULL, NULL,
                            &startup, &process)) {
            fprintf(stderr, "FAIL: launch %s (Windows error %lu)\n", names[i], GetLastError());
            return 1;
        }
        search.pid = process.dwProcessId;
        search.window = NULL;
        for (attempt = 0; attempt < 500; ++attempt) {
            EnumWindows(FindSample, (LPARAM)&search);
            if (search.window || WaitForSingleObject(process.hProcess, 0) == WAIT_OBJECT_0)
                break;
            Sleep(20);
        }
        if (!search.window) {
            fprintf(stderr, "FAIL: %s did not open a visible window\n", names[i]);
            TerminateProcess(process.hProcess, 1);
            CloseHandle(process.hThread);
            CloseHandle(process.hProcess);
            return 1;
        }
        Sleep(100);
        PostMessage(search.window, WM_CLOSE, 0, 0);
        if (WaitForSingleObject(process.hProcess, 10000) != WAIT_OBJECT_0) {
            fprintf(stderr, "FAIL: %s did not exit after WM_CLOSE\n", names[i]);
            TerminateProcess(process.hProcess, 1);
            CloseHandle(process.hThread);
            CloseHandle(process.hProcess);
            return 1;
        }
        if (!GetExitCodeProcess(process.hProcess, &exitCode)) exitCode = 1;
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        if (exitCode != 0) {
            fprintf(stderr, "FAIL: %s exited with %lu\n", names[i], exitCode);
            return 1;
        }
        printf("PASS: %s opens and exits on window closure\n", names[i]);
    }
    return 0;
}
