cmake_minimum_required(VERSION 3.1)
project(MigangBotCPP)
set(CMAKE_CXX_STANDARD 20)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin/debug)  
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin/release)  

find_package(Threads REQUIRED) # pthread

find_package(PkgConfig REQUIRED) # libjsoncpp
pkg_check_modules(JSONCPP jsoncpp)

link_libraries(${JSONCPP_LIBRARIES})

include_directories(Externals)
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
                
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads ${JSONCPP_LIBRARIES})