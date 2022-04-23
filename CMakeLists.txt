cmake_minimum_required(VERSION 3.12)
project(MigangBot)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BUILD_SHARED_LIBS OFF)

option(BUILD_TEST "build test" ON)

find_package(Threads REQUIRED) # pthread

# Boost
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED)
IF(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
ENDIF(Boost_FOUND)

# opencv
find_package(OpenCV REQUIRED)
IF(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
ENDIF(OpenCV_FOUND)

# mysql
include(FindPkgConfig)
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)
link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})

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
add_subdirectory(third-party/libhv)

# oneTBB
set(TBB_TEST OFF CACHE INTERNAL "Enable testing")
add_subdirectory(third-party/oneTBB)

# fmt
add_subdirectory(third-party/fmt)

aux_source_directory(Sources/message MESSAGE_SRC)

add_executable(${PROJECT_NAME} 
                Sources/main.cpp
                third-party/cpp-base64/base64.cpp
                ${MESSAGE_SRC}
)
target_include_directories(${PROJECT_NAME} PRIVATE
                            Sources 
                            third-party/cpp-base64
                            ${LIBMYSQLCLIENT_INCLUDE_DIRS}
)    

target_link_libraries(${PROJECT_NAME} PRIVATE 
                        Threads::Threads 
                        yaml-cpp 
                        nlohmann_json::nlohmann_json 
                        hv_static 
                        spdlog
                        TBB::tbb
                        fmt
                        ${LIBMYSQLCLIENT_LIBRARIES}
                        ${OpenCV_LIBS}
)

IF(BUILD_TEST)
    add_subdirectory(test)
ENDIF()