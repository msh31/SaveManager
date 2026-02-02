#!/bin/bash
set -e
BUILD_DIR="build-windows"

echo "Building SaveManager for Windows (cross-compilation)..."
echo ""

if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "Error: mingw-w64 is not installed."
    exit 1
fi

VCPKG_DETECTED=""
if [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_DETECTED="$VCPKG_ROOT"
    echo "Using vcpkg from VCPKG_ROOT: $VCPKG_ROOT"
elif [ -f "$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_DETECTED="$(pwd)/vcpkg"
    export VCPKG_ROOT="$VCPKG_DETECTED"
    echo "Using vcpkg from: $VCPKG_DETECTED"
elif [ -f "$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_DETECTED="$HOME/vcpkg"
    export VCPKG_ROOT="$VCPKG_DETECTED"
    echo "Using vcpkg from: $VCPKG_DETECTED"
else
    echo "Warning: vcpkg not found!"
    exit 1
fi

echo "Installing dependencies for Windows target..."
"$VCPKG_DETECTED/vcpkg" install --triplet=x64-mingw-static

echo "Configuring build..."
cmake -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE=toolchains/toolchain-mingw-w64-vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DVCPKG_APPLOCAL_DEPS=OFF

echo ""
echo "Building..."
cmake --build "$BUILD_DIR" -j$(nproc)

echo ""
if [ -f "$BUILD_DIR/savemanager.exe" ]; then
    echo "Build complete! Windows executable: $BUILD_DIR/savemanager.exe"
else
    echo "Build complete! Executable: $BUILD_DIR/savemanager"
fi
