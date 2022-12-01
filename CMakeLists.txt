cmake_minimum_required(VERSION 2.8)
project(pp-edge-dnp3)
add_executable(${PROJECT_NAME} src/main.cpp src/mqtt_client.cpp src/dnp3.cpp)
#set(CMAKE_CXX_FLAGS "-std=c++11 -O3")

file(GLOB SOURCES src/*.cpp)
include_directories(include)
include_directories(/usr/local/include)
include_directories(/home/fvallejo/projects/tahu/c/core/include)
#find_library(LIBOPENDP3 libopendnp3.a)
#add_library(${PROJECT_NAME} /usr/local/lib/libopendnp3.a)

target_link_libraries(${PROJECT_NAME} PRIVATE /usr/local/lib/libopendnp3.a -lmosquitto)
