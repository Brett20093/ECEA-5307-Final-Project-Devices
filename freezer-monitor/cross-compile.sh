#!/usr/bin/env bash

# The first argument needs to be the path to the buildroot output folder

make clean
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-