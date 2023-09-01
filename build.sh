#!/bin/bash
set -e
echo=echo

# Set Color
CSI=$($echo -e "\033[")
CEND="${CSI}0m"
CDGREEN="${CSI}32m"
CRED="${CSI}1;31m"
CGREEN="${CSI}1;32m"
CYELLOW="${CSI}1;33m"
CBLUE="${CSI}1;34m"
CMAGENTA="${CSI}1;35m"
CCYAN="${CSI}1;36m"
CSUCCESS="$CDGREEN"
CFAILURE="$CRED"
CQUESTION="$CMAGENTA"
CWARNING="$CYELLOW"
CMSG="$CCYAN"


PWD=$(dirname $(realpath "$0"))
CURRENT_DIR=$PWD

if [ ! -d build ]; then
    mkdir build
else
    rm -rf build/*
fi

# BUILD_CONFIG="${BUILD_CONFIG} -DSCAN=ON"
export PATH=/usr/local/toolchains/linaro-aarch64-2018.08-gcc8.2/bin:$PATH

BUILD_CONFIG="-DCMAKE_BUILD_TYPE=Debug"


if [ "$(arch)" == "x86_64" ]; then
    BUILD_CONFIG="${BUILD_CONFIG} -DCMAKE_TOOLCHAIN_FILE=${CURRENT_DIR}/helper/cmake/linux-aarch64-gcc9.2.cmake"
else
    BUILD_CONFIG="${BUILD_CONFIG}"
fi

echo ${BUILD_CONFIG}

cd build
nproc=$(nproc)

cmake .. ${BUILD_CONFIG}
make -j${nproc}

