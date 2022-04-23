add_executable(pressure_test pressure_test.cpp)

target_include_directories(pressure_test PRIVATE
                            ${CMAKE_BINARY_DIR}/third-party/libhv/include
                            ${CMAKE_SOURCE_DIR}/third-party/json/include
)

target_link_libraries(pressure_test
                    hv_static
                    nlohmann_json::nlohmann_json 
)