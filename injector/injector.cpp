// DLL injector for Homefront freecam mod.
// via CreateRemoteThread + LoadLibraryW.

#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <string>

static const wchar_t* TARGET_PROCESS = L"HOMEFRONT.exe";

static DWORD FindProcessId()
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    DWORD pid = 0;

    if (Process32FirstW(snap, &pe))
    {
        do {
            if (_wcsicmp(pe.szExeFile, TARGET_PROCESS) == 0)
            {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }

    CloseHandle(snap);
    return pid;
}

static std::wstring GetDllPath()
{
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    std::wstring path(exePath);
    size_t slash = path.find_last_of(L"\\/");
    if (slash != std::wstring::npos)
        path = path.substr(0, slash + 1);

    path += L"homefront_freecam.dll";
    return path;
}

int wmain()
{
    std::wstring dllPath = GetDllPath();

    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        wprintf(L"[!] DLL not found: %s\n", dllPath.c_str());
        wprintf(L"    Place homefront_freecam.dll next to injector.exe.\n");
        system("pause");
        return 1;
    }

    wprintf(L"[*] Waiting for %s ...\n", TARGET_PROCESS);

    DWORD pid = 0;
    for (int i = 0; i < 120; ++i)
    {
        pid = FindProcessId();
        if (pid) break;
        Sleep(500);
    }

    if (!pid)
    {
        wprintf(L"[!] Game process not found. Start the game first.\n");
        system("pause");
        return 1;
    }

    wprintf(L"[*] Found process, PID = %lu\n", pid);

    HANDLE hProc = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, pid);

    if (!hProc)
    {
        wprintf(L"[!] OpenProcess failed. Run injector.exe as Administrator.\n");
        system("pause");
        return 1;
    }

    SIZE_T bytes = (dllPath.size() + 1) * sizeof(wchar_t);
    LPVOID remoteMem = VirtualAllocEx(hProc, nullptr, bytes,
                                      MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMem)
    {
        wprintf(L"[!] VirtualAllocEx failed.\n");
        CloseHandle(hProc);
        system("pause");
        return 1;
    }

    WriteProcessMemory(hProc, remoteMem, dllPath.c_str(), bytes, nullptr);

    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    auto loadLib = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        GetProcAddress(kernel32, "LoadLibraryW"));

    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, loadLib, remoteMem, 0, nullptr);
    if (!hThread)
    {
        wprintf(L"[!] CreateRemoteThread failed.\n");
        VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProc);
        system("pause");
        return 1;
    }

    WaitForSingleObject(hThread, 10000);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);
    if (exitCode == 0)
        wprintf(L"[!] LoadLibraryW returned 0 - DLL may not have loaded.\n");
    else
        wprintf(L"[+] DLL loaded successfully.\n");

    CloseHandle(hThread);
    VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
    CloseHandle(hProc);

    wprintf(L"[*] In-game: F9 = toggle freecam, F10 = unload mod.\n");
    system("pause");
    return 0;
}
