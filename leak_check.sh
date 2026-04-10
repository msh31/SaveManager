#!/bin/bash

podman build -t savemanager:valgrind -f Dockerfile.valgrind .

podman run --rm -it \
  -v "$PWD:/code" \
  -v "$HOME/.conan2:/root/.conan2" \
  savemanager:valgrind \
  bash -c "
    cd /code && \
    conan install . --output-folder=build-valgrind --build=missing && \
    cmake -B build-valgrind -G Ninja -DCMAKE_TOOLCHAIN_FILE=build-valgrind/conan_toolchain.cmake && \
    cmake --build build-valgrind && \
    valgrind --leak-check=full ./build-valgrind/savemanager
  "
