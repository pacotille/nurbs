# Cube Demo — ImGui + SDL2 + OpenGL + CMake

Un cube 3D interactif rendu en OpenGL avec une interface ImGui.

## Dépendances système

### Ubuntu / Debian
```bash
sudo apt install libsdl2-dev libgl1-mesa-dev cmake build-essential git
```

### macOS (Homebrew)
```bash
brew install sdl2 cmake
```

### Windows (vcpkg)
```powershell
vcpkg install sdl2 opengl
```

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
./CubeDemo
```

> ImGui est téléchargé automatiquement via CMake FetchContent (v1.90.4).

## Fonctionnalités

- **Cube coloré** avec 6 faces de couleurs différentes et éclairage diffus
- **Rotation automatique** avec vitesse réglable sur X, Y, Z
- **Mode manuel** pour contrôler les angles précisément
- **Fil de fer** (wireframe) activable avec transparence réglable
- **FOV et distance caméra** ajustables en temps réel
- **Couleur de fond** personnalisable
- **FPS** et delta-time affichés

## Contrôles

Tout se contrôle depuis la fenêtre ImGui.  
`ESC` pour quitter.

## Architecture

```
cube_project/
├── CMakeLists.txt       # Build system, FetchContent pour ImGui
└── src/
    └── main.cpp         # Tout en un seul fichier :
                         #   - Maths (matrices 4x4, perspective, rotations)
                         #   - Géométrie du cube
                         #   - Rendu OpenGL fixed-function
                         #   - UI ImGui
```
