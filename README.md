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

For contributing: Please make sure to add any additional `.cpp` files to `add_executables` in `CMakeLists.txt`. The build command stays the same regardless.