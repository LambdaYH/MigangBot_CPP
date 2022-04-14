cmake_minimum_required(VERSION 3.1)
project(MigangBotCPP)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Threads REQUIRED) # pthread

# Boost
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost)
IF(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
ENDIF(Boost_FOUND)

add_subdirectory(third-party/yaml-cpp)
set(JSON_ImplicitConversions OFF)
add_subdirectory(third-party/json)
add_subdirectory(third-party/spdlog)

include_directories(Sources)
include_directories(
    Sources
    third-party/json/include
    third-party/yaml-cpp/include
    third-party/spdlog/include
)

aux_source_directory(Sources/bot BOT_SRC)
aux_source_directory(Sources/event EVENT_SRC)
aux_source_directory(Sources/logger LOGGER_SRC)

add_executable(${PROJECT_NAME} 
                Sources/main.cpp
                ${BOT_SRC}
                ${LOGGER_SRC}
                ${EVENT_SRC}
)
                
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads yaml-cpp nlohmann_json::nlohmann_json spdlog)