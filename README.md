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
./build/dsl generated-slice-add/slice-add.dsl generated-slice-add/
```

```bash
./build/compiler (Runs the main in compiler.cpp)
```

```bash
chmod +x my-install
./run.sh <Program Name (GoL)> [Cell Size (Number)]
```

### Example
```bash
bash run.sh GoL 3
```
