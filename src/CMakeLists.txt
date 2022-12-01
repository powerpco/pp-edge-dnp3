add_executable(${PROJECT_NAME} main.cpp mqtt_client.cpp dnp3.cpp)

add_subdirectory(tahu)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/deps/tahu/c/core/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE opendnp3 mosquitto tahu)
