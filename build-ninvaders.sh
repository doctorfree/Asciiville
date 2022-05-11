#!/bin/bash

cd games/ninvaders
cmake -B cmake_build
cmake --build cmake_build -j2
