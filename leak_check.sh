#!/bin/bash

podman build -t savemanager:valgrind -f Dockerfile.valgrind .

podman run --rm -it \
  -v "$PWD:/code" \
  savemanager:valgrind \
  bash -c "
    cd /code && \
    cmake -B build-valgrind -G Ninja && \
    cmake --build build-valgrind && \
    valgrind --leak-check=full ./build-valgrind/savemanager
  "
