#include "input.h"
#include <array>

static std::array<bool, 256> s_cur  = {};
static std::array<bool, 256> s_prev = {};

void Input::Update()
{
    s_prev = s_cur;
    for (int i = 0; i < 256; ++i)
        s_cur[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
}

bool Input::IsHeld(int vk)
{
    if (vk < 0 || vk >= 256) return false;
    return s_cur[static_cast<size_t>(vk)];
}

bool Input::JustPressed(int vk)
{
    if (vk < 0 || vk >= 256) return false;
    auto idx = static_cast<size_t>(vk);
    return s_cur[idx] && !s_prev[idx];
}
