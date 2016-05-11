#!/bin/bash

# This script assumes that you have your directory set up as follows:
# | Parent Directory
#   | llvm
#     | clang-plugin
#       | build.sh (this script)
#   | build_llvm
#
# Feel free to adjust these paths to fit your needs.
PWD=`pwd`/`dirname $0`
OUT_DIR=${PWD}/../../build_llvm
LLVM_DIR=${PWD}/..

mkdir -p ${OUT_DIR} && cd ${OUT_DIR}
cmake -G Ninja -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_C_COMPILER="${HOME}/goma/clang" -DCMAKE_CXX_COMPILER="${HOME}/goma/clang++" -DBUILD_SHARED_LIBS=ON ${LLVM_DIR}

ninja -j40
ninja -j40 libclangPlugin

cd ${PWD}

