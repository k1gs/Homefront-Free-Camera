#pragma once
#include <Windows.h>

namespace Input
{
    void Update();
    bool IsHeld(int vk);
    bool JustPressed(int vk);
}
