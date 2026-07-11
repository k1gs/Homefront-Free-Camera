#pragma once
#include <Windows.h>
#include <cstdio>
#include <cstdarg>

// here is a simple logger for debugging camera in mod
namespace Log
{
    inline HMODULE g_selfModule = nullptr;

    inline void BuildPath(char* out, size_t cap)
    {
        char dllPath[MAX_PATH] = {};
        GetModuleFileNameA(g_selfModule, dllPath, MAX_PATH);

        char* lastSlash = nullptr;
        for (char* p = dllPath; *p; ++p)
            if (*p == '\\' || *p == '/') lastSlash = p;
        if (lastSlash) *(lastSlash + 1) = '\0';

        _snprintf_s(out, cap, _TRUNCATE, "%sfreecam.log", dllPath);
    }

    inline void Write(const char* fmt, ...)
    {
        static bool s_first = true;
        char path[MAX_PATH] = {};
        BuildPath(path, sizeof(path));

        FILE* f = nullptr;
        fopen_s(&f, path, s_first ? "w" : "a");
        if (!f) return;
        s_first = false;

        va_list args;
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        va_end(args);

        fputc('\n', f);
        fclose(f);
    }
}
