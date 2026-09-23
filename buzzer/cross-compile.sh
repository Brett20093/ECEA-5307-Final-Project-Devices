#!/usr/bin/env bash

# The first argument needs to be the path to the buildroot output folder

make clean
make -C ${1%/}/build/linux-* ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- M=$(pwd) V=1 modules