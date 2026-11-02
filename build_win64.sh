#!/bin/bash
set -e

# Build directory
BUILD_DIR="build_win64"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Detect if we are on Arch Linux to set DLL source path
DLL_SOURCE_DIR="/usr/x86_64-w64-mingw32/bin"
if [ -f /etc/arch-release ]; then
    echo "Arch Linux detected, using $DLL_SOURCE_DIR for DLLs"
else
    echo "Non-Arch Linux detected, please ensure $DLL_SOURCE_DIR is correct"
fi

# Configure using the toolchain file
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE=mingw-w64.cmake \
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build "$BUILD_DIR" -j$(nproc)

# Copy required DLLs to the build directory
echo "Copying MinGW DLLs to build directory..."
DLLS=("libstdc++-6.dll" "libgcc_s_seh-1.dll" "libwinpthread-1.dll")

for dll in "${DLLS[@]}"; do
    if [ -f "$DLL_SOURCE_DIR/$dll" ]; then
        cp "$DLL_SOURCE_DIR/$dll" "$BUILD_DIR/"
        echo "Copied $dll"
    else
        echo "Warning: $dll not found in $DLL_SOURCE_DIR"
    fi
done

echo "Build complete. Executables and DLLs are in $BUILD_DIR"
