# MediCore Hospital Management System (SFML GUI)

## Repository

Add your public GitHub repository URL here (required by course submission).

## Build (Windows)

1. Install [CMake](https://cmake.org/download/) and a C++20 toolchain (Visual Studio 2022 with **Desktop development with C++**, or MinGW with CMake).
2. From this project directory:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

3. Run `build/Release/MediCore.exe` (or `MediCore` from the IDE). The post-build step copies `assets/` and the `.txt` data files next to the executable so paths resolve.

If CMake cannot find SFML, ensure `SFML_STATIC_LIBRARIES` is `ON` (set in `CMakeLists.txt`) and that this folder is the prefix containing `lib/cmake/SFML/`.

## Assets (sprites)

Place these PNG files in the `assets/` folder (paths are built generically in `MediCoreGui.cpp`):

- `Main_page.png` — role selection / login backgrounds  
- `Patient_service.png` — patient session  
- `Doctor_console.png` — doctor session  
- `Admin_panel.png` — admin session  

Optional: `assets/font.ttf` for typography. If it is missing, the app tries `C:/Windows/Fonts/arial.ttf`.

## Data files

All persistence uses comma-separated `.txt` files in the working directory (same folder as the executable after build). Do not hardcode user data; edit the provided seed files or use the in-app flows.

## Entry point

The graphical application entry point is `MediCoreGui.cpp` (`main`). Legacy console-oriented domain logic remains in `Backend.cpp` for reference; the GUI uses `MediCoreGuiAPI.cpp` for buffer-based actions and `FileHandler` for all file I/O.
