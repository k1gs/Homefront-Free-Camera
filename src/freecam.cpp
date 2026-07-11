#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "freecam.h"
#include "input.h"
#include <Windows.h>
#include <cmath>

FreecamState g_fc;

static constexpr float UU_TO_RAD = 3.14159265358979f / 32768.f;

static void GetAxes(const FRotator& rot, FVector& fwd, FVector& right, FVector& up)
{
    float yaw   = static_cast<float>(rot.Yaw)   * UU_TO_RAD;
    float pitch = static_cast<float>(rot.Pitch) * UU_TO_RAD;
    float cp = cosf(pitch), sp = sinf(pitch);
    float cy = cosf(yaw),   sy = sinf(yaw);

    fwd.X = cp * cy;
    fwd.Y = cp * sy;
    fwd.Z = sp;

    right.X = -sy;
    right.Y =  cy;
    right.Z =  0.f;

    up.X = 0.f;
    up.Y = 0.f;
    up.Z = 1.f;
}

void Freecam::Toggle()
{
    g_fc.enabled = !g_fc.enabled;
    g_fc.haveLoc = false;
    g_fc.haveRot = false;
}

void Freecam::Reset()
{
    g_fc.enabled = false;
    g_fc.haveLoc = false;
    g_fc.haveRot = false;
}

void Freecam::CaptureLocation(const FVector& v)
{
    if (!g_fc.haveLoc) { g_fc.pos = v; g_fc.haveLoc = true; }
}

void Freecam::CaptureRotation(const FRotator& r)
{
    if (!g_fc.haveRot) { g_fc.rot = r; g_fc.haveRot = true; }
}

void Freecam::Tick(float dt)
{
    if (!g_fc.enabled) return;
    if (dt > 0.1f)  dt = 0.1f;
    if (dt <= 0.f)  dt = 0.016f;

    float speed = g_fc.moveSpeed;
    if (Input::IsHeld(VK_SHIFT)) speed *= g_fc.fastMult;
    if (Input::IsHeld(VK_MENU))  speed *= g_fc.slowMult;
    float dist = speed * dt;

    FVector fwd, right, up;
    GetAxes(g_fc.rot, fwd, right, up);

    if (Input::IsHeld('W'))        g_fc.pos = g_fc.pos + fwd   * dist;
    if (Input::IsHeld('S'))        g_fc.pos = g_fc.pos + fwd   * (-dist);
    if (Input::IsHeld('A'))        g_fc.pos = g_fc.pos + right * (-dist);
    if (Input::IsHeld('D'))        g_fc.pos = g_fc.pos + right * dist;
    if (Input::IsHeld('Q') ||
        Input::IsHeld(VK_SPACE))   g_fc.pos = g_fc.pos + up    * dist;
    if (Input::IsHeld('E') ||
        Input::IsHeld(VK_CONTROL)) g_fc.pos = g_fc.pos + up    * (-dist);

    int rotDelta = static_cast<int>(g_fc.rotSpeed * dt);
    if (Input::IsHeld(VK_LEFT))  g_fc.rot.Yaw   -= rotDelta;
    if (Input::IsHeld(VK_RIGHT)) g_fc.rot.Yaw   += rotDelta;
    if (Input::IsHeld(VK_UP))    g_fc.rot.Pitch += rotDelta;
    if (Input::IsHeld(VK_DOWN))  g_fc.rot.Pitch -= rotDelta;

    const int PITCH_LIMIT = 16000;
    if (g_fc.rot.Pitch >  PITCH_LIMIT) g_fc.rot.Pitch =  PITCH_LIMIT;
    if (g_fc.rot.Pitch < -PITCH_LIMIT) g_fc.rot.Pitch = -PITCH_LIMIT;
}

void Freecam::WriteLocation(FVector* out)
{
    if (out && g_fc.enabled && g_fc.haveLoc) *out = g_fc.pos;
}

void Freecam::WriteRotation(FRotator* out)
{
    if (out && g_fc.enabled && g_fc.haveRot) *out = g_fc.rot;
}
