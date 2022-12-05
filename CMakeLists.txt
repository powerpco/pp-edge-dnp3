cmake_minimum_required(VERSION 3.22.1)

project(pp-edge-dnp3)

add_subdirectory(deps/libconfig)
add_subdirectory(src)

#install(FILES ${CMAKE_BINARY_DIR}/libconfig++.a DESTINATION lib)
install(FILES ${CMAKE_BINARY_DIR}/libtahu.a DESTINATION lib)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME} DESTINATION bin PERMISSIONS WORLD_EXECUTE)
