# Homefront Freecam Mod

<img width="1920" height="1080" alt="20260711214518_1" src="https://github.com/user-attachments/assets/7be15de4-ffc0-4a85-bbf5-8f45984aa7d0" />
<img width="1920" height="1080" alt="20260711214505_1" src="https://github.com/user-attachments/assets/4f009823-e39a-4c4b-b8f9-a581b1289c04" />
<img width="1920" height="1080" alt="20260711214451_1" src="https://github.com/user-attachments/assets/169712c1-dd1b-41fe-b179-e48ce2eb851f" />


The project has two binaries:

- `homefront_freecam.dll` - the injected module that hooks the in-game camera function and drives the free camera.
- `injector.exe` - a small launcher that finds `HOMEFRONT.exe` and injects the DLL with `CreateRemoteThread + LoadLibraryW`.

The codebase is written in C++17, built with CMake, and uses a local `MinHook` dependency under `third_party/minhook`.

## What It Does

The mod intercepts the native game function that calculates the player's camera position and rotation every frame. After the original game code runs, the mod:

1. Reads the normal camera output from the game.
2. Captures that position and rotation as the freecam starting point on the first frame after enable.
3. Updates its own camera state from keyboard input.
4. Overwrites the function output with the freecam position and rotation.
5. Freezes the pawn in place while freecam is active so the body does not drift with movement input.

That gives you a detached camera while the rest of the game continues to run normally.

## Project Layout

- [`src/dllmain.cpp`](D:/proj/Fcam/src/dllmain.cpp) starts the worker thread after DLL injection, waits for startup, initializes hooks, and handles hotkeys.
- [`src/hooks.cpp`](D:/proj/Fcam/src/hooks.cpp) installs the hook on `GetActorEyesViewPoint`, computes frame delta time, writes custom camera values, and freezes the pawn.
- [`src/freecam.cpp`](D:/proj/Fcam/src/freecam.cpp) stores freecam state, converts `FRotator` to movement axes, and applies movement and rotation input.
- [`src/input.cpp`](D:/proj/Fcam/src/input.cpp) polls keyboard state through `GetAsyncKeyState`.
- [`injector/injector.cpp`](D:/proj/Fcam/injector/injector.cpp) waits for `HOMEFRONT.exe`, allocates memory in the target process, and loads the DLL remotely.

## How It Works

### Camera Hook

The current implementation hooks a hardcoded native function address:

- target virtual address: `0x00A52070`
- rebased against executable image base: `0x00400000`

This means the mod is tied to a specific `Homefront` executable build. If the game binary changes because of a patch, another release, or a different distribution, the hook address may need to be updated.

### Pawn Freeze

While freecam is active, the mod writes directly to the pawn position fields to keep the player body fixed in place:

- `0xBC`, `0xC0`, `0xC4` for `X/Y/Z`

These offsets are also version-specific.

## In-Game Controls

- `F9` - toggle freecam on or off
- `F10` - unload the DLL from the game process
- `W`, `A`, `S`, `D` - move forward, left, backward, right
- `Q` or `Space` - move up
- `E` or `Ctrl` - move down
- `Left`, `Right` - yaw
- `Up`, `Down` - pitch
- `Shift` - move faster
- `Alt` - move slower

## Quick Start

1. Build `homefront_freecam.dll` and `injector.exe`.
2. Make sure both files are in the same directory.
3. Start `Homefront`.
4. Run `injector.exe`, preferably as Administrator if needed.
5. Wait for the successful DLL load message.
6. Press `F9` in game to enable freecam.

For build steps, see [BUILD.md](D:/proj/Fcam/BUILD.md).

## Requirements

- Windows
- CMake 3.20 or newer
- Visual Studio 2022 or another MSVC toolchain with x86 support
- `Win32` / `x86` build target
- `MinHook` checked out under `third_party/minhook`

The project intentionally stops configuration if it is not being built as 32-bit.

## Build Outputs

After a successful `Release` build, the expected outputs are:

- `build/Release/homefront_freecam.dll`
- `build/Release/injector.exe`

Depending on the Visual Studio generator, output paths may also appear under `build/Win32/Release/...`.

## Limitations

- Hardcoded to a specific `HOMEFRONT.exe` build.
- Uses `CreateRemoteThread + LoadLibraryW`, so antivirus or OS protections may interfere with injection.
- If the game runs elevated, the injector usually needs to run elevated too.
- There is no pattern scanning or automatic signature resolution yet. Hook addresses and offsets must be updated manually.

## Debugging

The repository includes a simple file logger in [`src/log.h`](D:/proj/Fcam/src/log.h) that writes `freecam.log` next to the DLL, but it is not currently used by the main code path. It can be wired in quickly if you want extra diagnostics for hook initialization or camera state.
