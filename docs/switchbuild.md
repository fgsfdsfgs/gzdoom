## Switch build instructions
1. Install devkitA64.
2. Install the following dependencies (you can use `[dkp-]pacman` in your devkitA64 environment for most of those):
* `libnx` >= 2.2.0
* `switch-sdl2` >= 2.0.8-19
* `switch-libjpeg-turbo`
* `switch-bzip2`
* `switch-zlib`
* `switch-mpg123`
* `switch-libogg`
* `switch-libvorbis`
* `switch-flac`
* `switch-libopus`
* [`openal-soft`](https://github.com/fgsfdsfgs/openal-soft)
* [`fluidsynth-lite`](https://github.com/fgsfdsfgs/fluidsynth-lite)
* `libsndfile` (you'll have to build it for the Switch manually)
3. Build GZDoom for your host platform.
4. Make a new directory, e.g. `switchbuild` in the repo directory, cd into it and execute the following commands:
```
source $DEVKITPRO/switchvars.sh
cmake -G"Unix Makefiles" \
-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/switch.cmake \
-DCMAKE_BUILD_TYPE=Release \
-DPKG_CONFIG_EXECUTABLE=$DEVKITPRO/portlibs/switch/bin/aarch64-none-elf-pkg-config \
-DIMPORT_EXECUTABLES=/path/to/your/native/build/ImportExecutables.cmake \
-DFORCE_CROSSCOMPILE=ON \
-DDYN_FLUIDSYNTH=OFF \
-DDYN_OPENAL=OFF \
-DDYN_MPG123=OFF \
-DDYN_SNDFILE=OFF \
-DNO_FMOD=ON \
-DNO_GTK=ON \
-DNO_OPENMP=ON \
..
```
where `/path/to/your/native/build` is the path to the directory containing the GZDoom build produced in step 3.
This should produce the executable, `gzdoom.nro`.
