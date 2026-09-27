# Marquee Console
> Authors: CHIU, Kristopher Lance, KE, Xan Luo, RAMIREZ, Diana Angela, YAMSUAN, Rhian Claire

A C++ CLI "OS emulator" featuring a command interpreter (input) and an animated text marquee (output).

The entry file where the main function is located is `src/main.cpp`.

## Build and run

CMake 3.16+ and a C++ compiler with support for C++17 (ex. g++) is required.

Initialise once:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
```
Use MinGW for Windows, otherwise you may omit

Build and run:

```powershell
cmake --build build
.\marquee.exe
```

## Running from an IDE

### Visual Studio Code (CMake Tools extension)
1. Open the project folder in VS Code.
2. Install the "CMake Tools" and "C/C++" extensions if you don't have them.
3. When prompted, select a kit (e.g., MinGW GCC or your installed compiler).
4. Open the Command Palette (Ctrl+Shift+P) → "CMake: Configure".
5. Click "Build" in the CMake Tools status bar, or run "CMake: Build" from the Command Palette.
6. Click "Run" (play icon) in the status bar, or open a terminal and run `.\marquee.exe` (Windows) / `./marquee` (Linux/macOS) from the project root.

For contributing: Please make sure to add any additional `.cpp` files to `add_executables` in `CMakeLists.txt`. The build command stays the same regardless.