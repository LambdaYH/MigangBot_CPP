cmake_minimum_required(VERSION 3.1)
project(MigangBotCPP)
set(CMAKE_CXX_STANDARD 20)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin/debug)  
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin/release)  

find_package(Threads REQUIRED) # pthread

set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost)
IF(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
ENDIF(Boost_FOUND)

add_subdirectory(third-party/yaml-cpp)
add_subdirectory(third-party/jsoncpp)
add_subdirectory(third-party/spdlog)

include_directories(Sources)

aux_source_directory(Sources/bot BOT_SRC)
aux_source_directory(Sources/event EVENT_SRC)
aux_source_directory(Sources/logger LOGGER_SRC)

add_executable(${PROJECT_NAME} 
                Sources/main.cpp
                ${BOT_SRC}
                ${LOGGER_SRC}
                ${EVENT_SRC}
)
                
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads jsoncpp yaml-cpp spdlog)