# Pathfinding
C++ implementation of the A*-pathfinding algorithm with a graphical user interface

# Building

## Requirements
- C++23 compatible compiler
- CMake 3.30+

## Steps

Configure and build

```powershell
cmake -B build
cmake --build ./build --config release
```

# Usage

## Legend

| Color | Meaning |
|---|---|
| Red | Start node |
| Light Green | Goal node |
| White | Empty node |
| Black | Wall node |
| Darker Green | Complete path |

## Controls
| Action | Controls | 
|---|---|
| Move start node | Shift + Left Click |
| Move goal node | Ctrl + Left Click |
| Place walls | Left Click + Drag |
| Erase walls | Right click + Drag |
| Run/Reset Pathfinding | Enter |

<br>

# Todo
- [ ] Display open/closed set
- [ ] Setting for node expansion in 4 or 8 directions
- [ ] Setting for heuristic function selection
- [ ] Add existing controls to GUI
- [ ] Setting for adjusting grid size
- [ ] Change colors to be nicer on the eyes
- [ ] Full path shouldn't obscure start/goal nodes
