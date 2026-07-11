#include "hooks.h"
#include "freecam.h"
#include "input.h"

#include <Windows.h>
#include <MinHook.h>
#include <cstdint>

// ---------------------------------------------------------------------------
// Hook target: native GetActorEyesViewPoint at VA 0x00A52070.
//
// This function is called every frame by the engine to compute the player's
// camera view (position + rotation). By calling the original and then
// overwriting the output buffers, we implement a detached free camera.
//
// Signature (from disassembly):
//   void __thiscall GetActorEyesViewPoint(APawn* this, FVector& outLoc, FRotator& outRot)
//   Prolog: push ebp; mov ebp,esp; and esp,-16; sub esp,0x74; ...
//   Ends with: ret 8 (two pointer args on stack)
//
// Detour convention: __fastcall(this, edx_unused, FVector*, FRotator*)
// ---------------------------------------------------------------------------

static constexpr uintptr_t VA_EYES = 0x00A52070;

using EyesFn = void(__fastcall*)(void* self, void* edx, FVector* outLoc, FRotator* outRot);
static EyesFn oEyes = nullptr;

// Frame timing via high-resolution performance counter.
static double   g_freq = 0.0;
static uint64_t g_last = 0;

static float FrameDt()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    uint64_t c = static_cast<uint64_t>(now.QuadPart);
    if (g_last == 0 || g_freq == 0.0) { g_last = c; return 0.016f; }
    double dt = static_cast<double>(c - g_last) / g_freq;
    g_last = c;
    if (dt <= 0.0) dt = 0.016;
    if (dt >  0.1) dt = 0.1;
    return static_cast<float>(dt);
}

// Pawn position freeze: keep the character in place while freecam is active.
static FVector g_frozenPawnPos = {};
static bool    g_pawnFrozen    = false;

static void FreezePawn(void* self)
{
    auto* base = reinterpret_cast<uint8_t*>(self);
    float* px = reinterpret_cast<float*>(base + 0xBC);
    float* py = reinterpret_cast<float*>(base + 0xC0);
    float* pz = reinterpret_cast<float*>(base + 0xC4);

    if (!g_pawnFrozen)
    {
        g_frozenPawnPos.X = *px;
        g_frozenPawnPos.Y = *py;
        g_frozenPawnPos.Z = *pz;
        g_pawnFrozen = true;
    }
    else
    {
        *px = g_frozenPawnPos.X;
        *py = g_frozenPawnPos.Y;
        *pz = g_frozenPawnPos.Z;
    }
}

static void UnfreezePawn()
{
    g_pawnFrozen = false;
}

static void __fastcall hkEyes(void* self, void* edx, FVector* outLoc, FRotator* outRot)
{
    // Let the game compute the normal view first.
    oEyes(self, edx, outLoc, outRot);

    if (g_fc.enabled)
    {
        // Capture the real camera state on the first frame after enabling.
        if (outLoc) Freecam::CaptureLocation(*outLoc);
        if (outRot) Freecam::CaptureRotation(*outRot);

        // Process movement/rotation input.
        Freecam::Tick(FrameDt());

        // Overwrite the output with freecam values.
        if (outLoc) Freecam::WriteLocation(outLoc);
        if (outRot) Freecam::WriteRotation(outRot);

        // Keep the pawn frozen in place.
        FreezePawn(self);
    }
    else if (g_pawnFrozen)
    {
        UnfreezePawn();
    }
}

static uintptr_t Rebased(uintptr_t va)
{
    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    return base + (va - 0x00400000);
}

bool Hooks::Init()
{
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    g_freq = static_cast<double>(f.QuadPart);

    if (MH_Initialize() != MH_OK)
        return false;

    uintptr_t addr = Rebased(VA_EYES);
    MH_STATUS s = MH_CreateHook(
        reinterpret_cast<LPVOID>(addr),
        reinterpret_cast<LPVOID>(&hkEyes),
        reinterpret_cast<LPVOID*>(&oEyes));

    if (s != MH_OK) { MH_Uninitialize(); return false; }
    if (MH_EnableHook(reinterpret_cast<LPVOID>(addr)) != MH_OK)
    {
        MH_RemoveHook(reinterpret_cast<LPVOID>(addr));
        MH_Uninitialize();
        return false;
    }

    return true;
}

void Hooks::Shutdown()
{
    Freecam::Reset();
    UnfreezePawn();
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}
