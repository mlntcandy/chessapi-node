cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_STANDARD 17)

project(chessapi-node)
include_directories(${CMAKE_JS_INC})
file(GLOB SOURCE_FILES "src/main.c" "src/chessapi/chessapi.c" "src/chessapi/bitboard.c")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

add_definitions(-DNAPI_VERSION=6)