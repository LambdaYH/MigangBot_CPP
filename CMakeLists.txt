cmake_minimum_required(VERSION 3.12)
project(MigangBot)
set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") #debug模式下开启-g选项
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif()

set(BUILD_SHARED_LIBS OFF)

option(BUILD_TEST "build test" ON)

# threads
find_package(Threads REQUIRED)

# mysql
include(FindPkgConfig)
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)
link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})

# opencv
find_package(OpenCV REQUIRED)
IF(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
ENDIF(OpenCV_FOUND)

# yaml-cpp
add_subdirectory(third-party/yaml-cpp)

# nlohmann/json
set(JSON_Install OFF CACHE INTERNAL "")
set(JSON_ImplicitConversions OFF CACHE INTERNAL "Enable implicit conversions.")
add_subdirectory(third-party/json)

# spdlog
add_subdirectory(third-party/spdlog)

# libhv
set(BUILD_SHARED OFF CACHE INTERNAL "build shared library")
set(BUILD_EXAMPLES OFF CACHE INTERNAL "build examples")
set(BUILD_UNITTEST OFF CACHE INTERNAL "build unittest")
set(WITH_OPENSSL ON CACHE INTERNAL "with openssl library")
add_subdirectory(third-party/libhv)

# oneTBB
set(TBB_TEST OFF CACHE INTERNAL "Enable testing")
add_subdirectory(third-party/oneTBB)

# fmt
add_subdirectory(third-party/fmt)

# utfcpp
add_subdirectory(third-party/utfcpp)

# zlib
add_subdirectory(third-party/zlib)

# hiredis
add_subdirectory(third-party/hiredis)

# lexbor
set(LEXBOR_BUILD_SHARED OFF CACHE INTERNAL "")
set(LEXBOR_BUILD_SEPARATELY OFF CACHE INTERNAL "")
add_subdirectory(third-party/lexbor)

# tencentcloud-sdk-cpp
set(BUILD_MODULES CACHE INTERNAL "nlp")
set(Tencentcloud-sdk-cpp_LIBS
    tencentcloud-sdk-cpp-nlp
    tencentcloud-sdk-cpp-core
    crypto
    curl
    uuid
)

add_executable(${PROJECT_NAME} 
                Sources/main.cpp
                third-party/cpp-base64/base64.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE
                            Sources 
                            third-party/cpp-base64
                            ${CMAKE_BINARY_DIR}/third-party/libhv/include
                            ${LIBMYSQLCLIENT_INCLUDE_DIRS}
)    

target_link_libraries(${PROJECT_NAME} PRIVATE 
                        Threads::Threads 
                        yaml-cpp 
                        nlohmann_json::nlohmann_json 
                        hv_static 
                        spdlog
                        fmt
                        ${OpenCV_LIBS}
                        TBB::tbb
                        libmysqlclient.a
                        zlibstatic
                        utf8::cpp
                        ${Tencentcloud-sdk-cpp_LIBS}
                        hiredis_static
                        lexbor_static
)

IF(BUILD_TEST)
    add_subdirectory(test)
ENDIF()