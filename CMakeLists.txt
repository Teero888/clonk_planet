cmake_minimum_required(VERSION 3.10)
project(ClonkPlanet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_definitions(-DLINUX -DC4ENGINE)

include_directories(
    ${CMAKE_SOURCE_DIR}/engine/inc
    ${CMAKE_SOURCE_DIR}/standard/inc
    ${CMAKE_SOURCE_DIR}/standard/unibase
    ${CMAKE_SOURCE_DIR}/standard/zlib
)

file(GLOB_RECURSE SRC_FILES
    "engine/src/*.cpp"
    "standard/src/*.cpp"
    "standard/unibase/*.c"
    "standard/unibase/*.cpp"
    "standard/src/Compat.cpp"
)

add_executable(clonk ${SRC_FILES})
target_link_libraries(clonk PRIVATE OpenGL::GL GLEW::GLEW glfw z m)
