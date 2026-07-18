# Homefront (2011) — Free Camera Mod

A DLL mod that adds a detachable free camera to Homefront (2011, Steam version).
Fly through levels, explore out-of-bounds areas, take screenshots from any angle.

## Building

Requirements: Visual Studio 2022, CMake 3.20+.

```powershell
git clone https://github.com/TsudaKageyu/minhook.git third_party/minhook
mkdir build
cd build
cmake .. -A Win32
cmake --build . --config Release
```

The `-A Win32` flag is mandatory — the game is 32-bit.

Output: `build/Release/homefront_freecam.dll` and `build/Release/injector.exe`.

## Usage

1. Launch Homefront and load into a level.
2. Place `injector.exe` and `homefront_freecam.dll` in the same folder.
3. Run `injector.exe` **as Administrator**.
4. The injector finds the game process and loads the DLL automatically.

## Controls

| Key | Action |
|-----|--------|
| F9 | Toggle free camera on/off |
| F10 | Unload the mod |
| W / A / S / D | Move forward / left / back / right |
| Arrow keys | Rotate camera |
| Q / Space | Move up |
| E / Ctrl | Move down |
| Shift | Fast movement (4x) |
| Alt | Slow movement (0.2x) |

When free camera is active, the player character is frozen in place.
Press F9 again to return to normal gameplay.

## How It Works

The mod hooks the native `GetActorEyesViewPoint` function (VA `0x00A52070`) which
the engine calls every frame to compute the player's view position and rotation.

When freecam is enabled:
- The original function runs first (computing the normal view).
- The output buffers (FVector location, FRotator rotation) are overwritten with
  freecam values computed from keyboard input.
- The pawn's world position is frozen by writing back the saved coordinates each frame.

The hook is installed via [MinHook](https://github.com/TsudaKageyu/minhook) using
the standard `__thiscall` → `__fastcall` detour pattern for x86.

## Compatibility

- **Tested on:** Homefront Steam version (HOMEFRONT.exe 1.0.0.1)
- **Architecture:** x86 (32-bit DLL)
- **ASLR:** The game loads at a fixed base (`0x00400000`, delta = 0), so static
  addresses work without relocation.

If the game crashes on inject, the hook address `0x00A52070` may not match your
executable version. You would need to locate `GetActorEyesViewPoint` via reverse
engineering (look for the function that calls `vtable[0x458]` on the player pawn
and reads location from `this+0xBC/0xC0/0xC4`).

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Build error "not x86" | Pass `-A Win32` to cmake |
| Access Denied on inject | Run injector.exe as Administrator |
| Game crashes immediately | Increase `STARTUP_DELAY_MS` in dllmain.cpp to 5000 |
| Camera doesn't respond | Ensure you're on a gameplay level, not in a menu |

## Project Structure

```
src/
├── dllmain.cpp     DLL entry point, input polling thread (F9/F10)
├── freecam.h/cpp   Camera math: position, rotation, input handling
├── hooks.h/cpp     MinHook detour on GetActorEyesViewPoint
└── input.h/cpp     Keyboard state via GetAsyncKeyState
injector/
└── injector.cpp    CreateRemoteThread + LoadLibraryW injector
CMakeLists.txt      Build configuration (Win32, static CRT)
```

## License

This project is provided as-is for educational and single-player modding purposes.
MinHook is used under the BSD 2-Clause license.
