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

## How to Build (Windows – MinGW)

### Build
If you have `make` installed (and the `Makefile` in the project root), simply run:

```bash
make or make -F MakeFile  

Then launch the exe!  
