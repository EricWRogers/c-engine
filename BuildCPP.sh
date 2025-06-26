#!/bin/bash

cmake -DCMAKE_BUILD_TYPE=Release -B build/
make -j32 -C build/
