# tint-wasm

## Overview
This repo contains a WASM32 compiled static library containing Google's [Tint](https://dawn.googlesource.com/tint) compiler for WGSL. It also contains a fully functional build of [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) since Tint depends on it!

I mostly worked on putting this together to assist in the development of WebGPU applications using SPIRV shaders regardless of the browser being used.

Also provided in this repo is `tint_wasm.cpp` a file that outlines a very simple API for cross compiling SPIR-V and WGSL using JavaScript + WASM.

### Features
**This library has only been compiled to support the *Tint SPIRV reader and writer*, as well as the *WGSL reader and writer*.**

## WASM Module
If you would like to simply convert SPIRV to WGSL using a few WASM calls in JS, I recommend using the `tint_wasm.cpp` API.

### Building the WASM Module:
#### Requirements
- [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)
- make

#### Build
Compiling the WASM module is easy:
```bash
cd tint-wasm
make
```
This will produce a `build/` directory that contains a simple WASM app to cross compile shaders from your browser, as well as the module that you can use with your own projects.

Take a look at shell.html to get a better understanding of how the API is used in Javascript. It is definitely more complex than most will ever need, but the underlying ideas of pointer management remain the same.

## Working With Just The Library
All necessary include files are provided in the root directory's subdirectories, and can be added to any project or `usr/local/include/tint/`.

```bash
## Make a tint directory in your target directory
sudo mkdir {target}/tint/

## Copy header files to a tint/ directory in your target directory
sudo cp -rp {api/,cmd/,lang/,spirv-tools/,utils/} {target}/tint/

## Make sure that you know where your static library is for linking purposes.
sudo cp libtint.a {target}/{location}/
```

### Linking
To link the library with a WASM project, ensure that you link using the `-L<path/to/directory> -ltint` flags with `emcc` or `clang`

## Compiling Tint From Source Using Emscripten 

### Step 1:
The following commands were used to generate the .o files used to compile the archive:
```bash
git clone https://dawn.googlesource.com/tint
cd tint
cp standalone.gclient .gclient
gclient sync
mkdir -p out/emsc_release/
cd out/emsc_release/
emcmake cmake ../.. -GNinja -DTINT_BUILD_SPV_READER=ON -DTINT_BUILD_CMD_TOOLS=ON -DTINT_BUILD_HLSL_READER=OFF -DTINT_BUILD_GLSL_READER=OFF -DTINT_BUILD_MSL_WRITER=OFF -DTINT_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
autoninja
```

#### Issues:
If you try to reproduce these steps and you see that the compilation process fails on protobuf with very few files left to compile, **do not worry**. This is expected since we aren't using `protobuf` to build the static libraries and the `protobuf` that was compiled won't work since we're using `emcc`. All of the object files needed to build Tint for web exist so we can use a bash script to collect all the object files and create an archive using `emar rcs`.

### Step 2:
The following bash script was used to compile the static library:

```bash
#! /usr/bin/bash

# Create a temporary directory for uniquely named object files
mkdir -p temp_objs

echo "Locating object files for Tint library..."

# Find all .o files in the output directory and copy them to the temp directory
find third_party/ -name "*.o" | sed '/\(_test\|mock\)/d' | while read -r obj_file; do
    # Create a unique name based on the file path (replacing '/' with '_')
    unique_name=$(echo "$obj_file" | sed 's/\//_/g')
    # Copy the file to the temp_objs directory with the unique name
    cp "$obj_file" "temp_objs/$unique_name"
done

find src/ -name "*.o" | sed '/\(_test\|mock\)/d' | while read -r obj_file; do
    # Create a unique name based on the file path (replacing '/' with '_')
    unique_name=$(echo "$obj_file" | sed 's/\//_/g')
    # Copy the file to the temp_objs directory with the unique name
    cp "$obj_file" "temp_objs/$unique_name"
done

echo "Creating static library for Tint..."

# Create the archive from the uniquely named object files
emar rcs libtint.a temp_objs/*.o

echo "Static library created: ${PWD}/libtint.a"

echo "Cleaning up..."

# Clean up the temporary directory
rm -r temp_objs
```
