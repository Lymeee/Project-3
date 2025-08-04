# Steam Review Comfort Visualizer

## Problem  
Steam reviews are hard to skim—users can’t quickly see overall sentiment or patterns in review text.

## Solution  
A graphical desktop app that lets you:

1. Load Steam review data from CSV  
2. Filter by game, recommendation, or vote count  
3. Search reviews and run BFS/DFS through connected reviews  
4. View results as a node-based graph

## Data  
Kaggle Steam Reviews  
https://www.kaggle.com/datasets/andrewmvd/steam-reviews  
Download here: [GitHub Release](https://github.com/Lymeee/Project-3/releases/tag/release) (Make sure this is in your project root)  

## Tools Used  
- **C++17**  
- **Dear ImGui** – UI rendering  
- **OpenGL** – Graphics backend  
- **GLFW** – Window/input handling  
- **Standard C++ STL** – Data structures and utilities  
- **CSV Parsing** – Done manually with `std::stringstream`

## How to Build

### Windows (MinGW)

Open a terminal in your project directory and run:

```bash
g++ main.cpp AdjacencyList.cpp ^
    thirdparty/imgui/imgui.cpp ^
    thirdparty/imgui/imgui_draw.cpp ^
    thirdparty/imgui/imgui_widgets.cpp ^
    thirdparty/imgui/imgui_tables.cpp ^
    thirdparty/imgui/backends/imgui_impl_glfw.cpp ^
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp ^
    -Ithirdparty/imgui ^
    -Ithirdparty/imgui/backends ^
    -std=c++17 -O2 ^
    -lglfw3 -lopengl32 -lgdi32 ^
    -o steamviz.exe
```

Then launch the app:

```bash
./steamviz.exe
```

---

### macOS

Make sure you have Xcode command line tools installed.  
Open a terminal in your project directory and run:

```bash
g++ main.cpp AdjacencyList.cpp \
    thirdparty/imgui/imgui.cpp \
    thirdparty/imgui/imgui_draw.cpp \
    thirdparty/imgui/imgui_widgets.cpp \
    thirdparty/imgui/imgui_tables.cpp \
    thirdparty/imgui/backends/imgui_impl_glfw.cpp \
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp \
    -Ithirdparty/imgui \
    -Ithirdparty/imgui/backends \
    -std=c++17 -O2 \
    -lglfw -framework OpenGL \
    -o steamviz
```

Then launch the app:

```bash
./steamviz
```

---

### Linux

Make sure you have `g++`, `libglfw3-dev`, and OpenGL development libraries installed.  
Open a terminal in your project directory and run:

```bash
g++ main.cpp AdjacencyList.cpp \
    thirdparty/imgui/imgui.cpp \
    thirdparty/imgui/imgui_draw.cpp \
    thirdparty/imgui/imgui_widgets.cpp \
    thirdparty/imgui/imgui_tables.cpp \
    thirdparty/imgui/backends/imgui_impl_glfw.cpp \
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp \
    -Ithirdparty/imgui \
    -Ithirdparty/imgui/backends \
    -std=c++17 -O2 \
    -lglfw -lGL -ldl -lpthread \
    -o steamviz
```

Then launch the app:

```bash
./steamviz
```

---

**Note:**  
- Make sure the `thirdparty` folder is included in your project directory.
- You may need to install dependencies (`GLFW`, `OpenGL`, etc.) using your package manager.
