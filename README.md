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

## Web Export

```bash
./scripts/build-web.sh web-release
python3 -m http.server 8000 --directory build-web-release/web

./scripts/build-web.sh web-debug
python3 -m http.server 8000 --directory build-web-debug/web
```

That bootstraps Emscripten and writes an HTML5 export to `build-web-release/web/`.
More detail: [Web export guide](docs/web-build.md)

## Ubuntu Dependencies

```bash
sudo apt install gcc gdb git clang ccache cmake libglm-dev libharfbuzz-dev libopusfile-dev libflac-dev libxmp-dev libfluidsynth-dev libwavpack-dev libbz2-dev libglew-dev podman patchelf lld fuse
```
