#!/bin/bash
export PATH=`pwd`/out/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu/bin/:$PATH
cd kernel/linux-4.9
make clean
cd -;
rm -rf out
