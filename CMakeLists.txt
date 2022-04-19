cmake_minimum_required(VERSION 3.12)
project(MigangBotCPP)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Threads REQUIRED) # pthread

# mysql
include(FindPkgConfig)
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)
link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})

# yaml-cpp
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(third-party/yaml-cpp)

# nlohmann/json
set(JSON_ImplicitConversions OFF CACHE BOOL "Enable implicit conversions.")
add_subdirectory(third-party/json)

# spdlog
add_subdirectory(third-party/spdlog)

# libhv
set(BUILD_SHARED OFF CACHE BOOL "build shared library")
set(BUILD_EXAMPLES OFF CACHE BOOL "build examples")
set(BUILD_UNITTEST OFF CACHE BOOL "build unittest")
add_subdirectory(third-party/libhv)

set(Beast_BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
set(Beast_BUILD_TESTS OFF CACHE BOOL "Build tests")
add_subdirectory(third-party/beast)

aux_source_directory(Sources/bot BOT_SRC)

add_executable(${PROJECT_NAME} 
                Sources/main.cpp
                ${BOT_SRC}
                ${LOGGER_SRC}
                ${EVENT_SRC}
)
target_include_directories(${PROJECT_NAME} PRIVATE
                            Sources 
                            third-party/json/include
                            third-party/yaml-cpp/include
                            third-party/beast/include
                            third-party/include
                            ${LIBMYSQLCLIENT_INCLUDE_DIRS}
)    

target_link_libraries(${PROJECT_NAME} PRIVATE 
                        Threads::Threads 
                        yaml-cpp 
                        nlohmann_json::nlohmann_json 
                        hv_static 
                        spdlog
                        ${LIBMYSQLCLIENT_LIBRARIES}
)