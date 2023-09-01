cmake_minimum_required(VERSION 2.8)
project(type4_main)

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/helper/cmake")
INCLUDE(color)
INCLUDE(version)
INCLUDE(function)

# EXECUTE_PROCESS(
#     COMMAND git rev-parse --short HEAD
#     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#     OUTPUT_VARIABLE SHORT_COMMIT
#     ERROR_VARIABLE VERSION_ERROR
#     RESULT_VARIABLE VERSION_RESULT
#     OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)

# IF(NOT VERSION_RESULT EQUAL 0)
#     MESSAGE(FATAL_ERROR "auto get version string failed")
# ENDIF()

# SET(VERSION_STRING
#     "v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${SHORT_COMMIT}")

# ADD_DEFINITIONS(-DTS_CONTAINER_VER="${VERSION_STRING}")
# MESSAGE(STATUS "TS container version is: ${VERSION_STRING}")

IF(SCAN)
    SET(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
    SET(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")
ENDIF()

IF(CODE_COVERAGE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()


if(APP_TYPE STREQUAL "official")
    add_definitions(-DOFFICIAL)
elseif(APP_TYPE STREQUAL "non-official")
    add_definitions(-DNON_OFFICIAL)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Wformat -Wformat-security -Wl,-z,relro,-z,now")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all -Wformat -Wformat-security -Wl,-z,relro,-z,now")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/lpr/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/lpr/include/flatbuffers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/lpr/include/flatcc_automatically_generated_header_files)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/json11)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/b64)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/MQTT-C/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/MQTT-C/templates)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src ALL_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/json11 ALL_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/MQTT-C/src MQTT_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/b64 B64_SRC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/opencv-4.4/linux-aarch64-gcc9.2/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/opencv-4.4/linux-aarch64-gcc9.2/lib)
set(OpenCV_LIBS opencv_core;opencv_imgcodecs;opencv_imgproc)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/lpr/lib)

set(TARGET type4_parking_online)

add_executable(${TARGET} ${MQTT_SRC} ${B64_SRC} ${ALL_SRC})
set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 11)
set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "-Wl,--allow-shlib-undefined")
target_link_libraries(${TARGET} ${OpenCV_LIBS} ts_lpr pthread)
