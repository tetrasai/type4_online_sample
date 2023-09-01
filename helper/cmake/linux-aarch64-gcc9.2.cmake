SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

SET(_CMAKE_TOOLCHAIN_LOCATION /opt/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin)
SET(CMAKE_C_COMPILER ${_CMAKE_TOOLCHAIN_LOCATION}/aarch64-none-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER ${_CMAKE_TOOLCHAIN_LOCATION}/aarch64-none-linux-gnu-g++)

SET(CROSS_COMPILE true)

SET(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})

SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
