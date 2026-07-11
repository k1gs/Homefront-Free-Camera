#include "hooks.h"
#include "freecam.h"
#include "input.h"

#include <Windows.h>

static HMODULE g_hModule = nullptr;
static volatile bool g_running = true;

static constexpr DWORD STARTUP_DELAY_MS = 3000;

static DWORD WINAPI MainThread(LPVOID)
{
    Sleep(STARTUP_DELAY_MS);

    bool hooked = Hooks::Init();

    while (g_running)
    {
        Input::Update();

        if (Input::JustPressed(VK_F9))
            Freecam::Toggle();

        if (Input::JustPressed(VK_F10))
        {
            g_running = false;
            break;
        }

        Sleep(8);
    }

    Freecam::Reset();
    if (hooked)
        Hooks::Shutdown();

    Sleep(100);
    FreeLibraryAndExitThread(g_hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        HANDLE h = CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        if (h) CloseHandle(h);
        break;
    }
    case DLL_PROCESS_DETACH:
        g_running = false;
        break;
    }
    return TRUE;
}
