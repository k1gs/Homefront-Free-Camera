#pragma once
#include <cstdint>
#include <cmath>

struct FVector
{
    float X, Y, Z;
    FVector() : X(0.f), Y(0.f), Z(0.f) {}
    FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}
    FVector operator+(const FVector& o) const { return { X+o.X, Y+o.Y, Z+o.Z }; }
    FVector operator-(const FVector& o) const { return { X-o.X, Y-o.Y, Z-o.Z }; }
    FVector operator*(float s)          const { return { X*s,   Y*s,   Z*s   }; }
};

// UE3 FRotator: Pitch/Yaw/Roll in "unreal units" (65536 = 360 degrees).
struct FRotator
{
    int Pitch, Yaw, Roll;
    FRotator() : Pitch(0), Yaw(0), Roll(0) {}
    FRotator(int p, int y, int r) : Pitch(p), Yaw(y), Roll(r) {}
};

struct FreecamState
{
    bool     enabled  = false;
    bool     haveLoc  = false;
    bool     haveRot  = false;
    FVector  pos      = {};
    FRotator rot      = {};
    float    moveSpeed = 300.f;
    float    fastMult  =   4.f;
    float    slowMult  =  0.2f;
    float    rotSpeed  = 40000.f;  // unreal units per second
};

extern FreecamState g_fc;

namespace Freecam
{
    void Toggle();
    void Reset();
    void Tick(float dt);
    void CaptureLocation(const FVector& v);
    void CaptureRotation(const FRotator& r);
    void WriteLocation(FVector* out);
    void WriteRotation(FRotator* out);
}
