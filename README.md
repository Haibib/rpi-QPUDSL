### Setup.

1. This project uses the [CMake](https://cmake.org/) build system. On macOS,
```bash
brew install cmake
```

2. Build `rpi-qpudsl`:

```bash
# Option 1: normal
cmake -S . -B build
cmake --build build -j<N PARALLELISM>

# Option 2: debug
cmake -S . -B build-dbg -DCMAKE_BUILD_TYPE=Debug
cmake --build build-dbg --config Debug -j<N PARALLELISM>
```

3. Run
```bash
./build/compiler
```