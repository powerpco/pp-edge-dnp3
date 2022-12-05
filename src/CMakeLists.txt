add_executable(${PROJECT_NAME} main.cpp mqtt_client.cpp dnp3.cpp settings.cpp)

add_subdirectory(tahu)
#find_library(cfg PATHS ../out/build/out NAMES config++)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/deps/tahu/c/core/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE opendnp3 mosquitto tahu config++)