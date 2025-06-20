#!/bin/bash

cmake -B build/
make -j8 -C build/
./dist/Linux/c-engine
