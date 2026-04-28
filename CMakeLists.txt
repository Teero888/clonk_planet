cmake_minimum_required(VERSION 3.30)
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
    ${CMAKE_SOURCE_DIR}/standard/inc/external
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")

add_executable(clonk ${SRC_FILES})
target_link_libraries(clonk PRIVATE OpenGL::GL GLEW::GLEW glfw z m)

add_executable(test_loading tests/test_loading.cpp)
target_link_libraries(test_loading PRIVATE z)

add_executable(test_zlib tests/test_zlib.cpp)
target_link_libraries(test_zlib PRIVATE z)

enable_testing()
add_test(NAME HeadlessTest 
         COMMAND ${CMAKE_COMMAND} -E env CLONK_TEST_HEADLESS=1 $<TARGET_FILE:clonk> Twonky.c4p
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
