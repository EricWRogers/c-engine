# c-engine

Custom C++ game engine and gameplay runtime built around:

- `CanisEngine` for engine code
- `GameCode` for gameplay code
- `c-engine` as the executable

## Documentation

- [Engine guide](docs/engine-guide.md)

## Build

```bash
cmake -S . -B build
cmake --build build -j4
```

## Run

```bash
./project/c-engine
```

## Ubuntu Dependencies

```bash
sudo apt install gcc gdb git clang ccache cmake libglm-dev libharfbuzz-dev libopusfile-dev libflac-dev libxmp-dev libfluidsynth-dev libwavpack-dev libbz2-dev libglew-dev podman patchelf lld fuse
```
