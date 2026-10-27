# Light Game

# Contents
- [Introduction](#introduction)
- [Compiling The Game](#compiling-the-game)
- [TODO](#todo)
- [License](#license)

### Build Status
![CI](https://github.com/skrolikowski/light-game/actions/workflows/ci.yml/badge.svg)

## Introduction

Learning C++, building a puzzle game involving lights.

## Compiling The Game

### Dependencies
- [Raylib](https://github.com/raysan5/raylib)
- [nlohmann/json](https://github.com/nlohmann/json) (fetched through CMake)

### MacOS
Navigate to the project root and run the following commands in the terminal:

```bash
cmake -S . -B build
cmake --build build
```

## TODO
- [ ] Undo / Redo controls
- [ ] Level Selection
- [ ] Scoring System (Zachtronics-like)
- [ ] Save Solutions
- [ ] Piece: Splitter (one input, multiple outputs)

## License

This project is licensed under the MIT License.
