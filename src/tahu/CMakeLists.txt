include_directories(${PROJECT_SOURCE_DIR}/deps/tahu/c/core/include)
add_library(tahu 
    ${PROJECT_SOURCE_DIR}/deps/tahu/c/core/src/pb_common.c
    ${PROJECT_SOURCE_DIR}/deps/tahu/c/core/src/pb_decode.c
    ${PROJECT_SOURCE_DIR}/deps/tahu/c/core/src/pb_encode.c
    ${PROJECT_SOURCE_DIR}/deps/tahu/c/core/src/tahu.c
    ${PROJECT_SOURCE_DIR}/deps/tahu/c/core/src/tahu.pb.c
)