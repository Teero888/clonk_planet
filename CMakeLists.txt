cmake_minimum_required(VERSION 3.30)
project(ClonkPlanet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

add_definitions(-DLINUX -DC4ENGINE)

include_directories(
    ${CMAKE_SOURCE_DIR}/engine/inc
    ${CMAKE_SOURCE_DIR}/standard/inc
    ${CMAKE_SOURCE_DIR}/standard/inc/external
    ${CMAKE_SOURCE_DIR}/standard/unibase
    ${CMAKE_SOURCE_DIR}/standard/zlib
    ${FREETYPE_INCLUDE_DIRS}
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

# Resource Management: Symlink data files to build directory
set(PLANET_DATA_DIR "${CMAKE_SOURCE_DIR}/planet_data")
file(GLOB DATA_FILES "${PLANET_DATA_DIR}/*")
set(DATA_SYMLINKS "")

foreach(DATA_FILE ${DATA_FILES})
    get_filename_component(DATA_FILENAME ${DATA_FILE} NAME)
    set(COPY_PATH "${CMAKE_BINARY_DIR}/${DATA_FILENAME}")
    add_custom_command(
        OUTPUT "${COPY_PATH}"
        COMMAND ${CMAKE_COMMAND} -E copy "${DATA_FILE}" "${COPY_PATH}"
        DEPENDS "${DATA_FILE}"
        COMMENT "Copying ${DATA_FILENAME}"
    )
    list(APPEND DATA_SYMLINKS "${COPY_PATH}")
endforeach()

add_custom_target(planet_data_sync ALL DEPENDS ${DATA_SYMLINKS})

add_executable(clonk ${SRC_FILES})
target_link_libraries(clonk PRIVATE OpenGL::GL GLEW::GLEW glfw z m ${FREETYPE_LIBRARIES})
add_dependencies(clonk planet_data_sync)

add_executable(test_loading tests/test_loading.cpp)
target_link_libraries(test_loading PRIVATE z)
add_dependencies(test_loading planet_data_sync)

add_executable(test_zlib tests/test_zlib.cpp)
target_link_libraries(test_zlib PRIVATE z)

add_executable(clonk_midi launcher/clonk_midi.cpp)
target_link_libraries(clonk_midi PRIVATE m dl pthread)
add_dependencies(clonk_midi planet_data_sync)

# enable_testing()
# add_test(NAME HeadlessTest 
#          COMMAND ${CMAKE_COMMAND} -E env CLONK_TEST_HEADLESS=1 $<TARGET_FILE:clonk> Twonky.c4p
#          WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(masterserver masterserver/masterserver.cpp)
target_link_libraries(masterserver pthread)
