cmake_minimum_required(VERSION 3.30)
project(ClonkPlanet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenGL REQUIRED)
set(GLEW_USE_STATIC_LIBS ON)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)
find_package(ZLIB REQUIRED)

if(UNIX)
    add_definitions(-DLINUX)
endif()
add_definitions(-DC4ENGINE)

if(MSVC)
    add_compile_options(/W4 /wd4996 /wd4100 /wd4244 /wd4305 /wd4505 /wd4189)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-sign-compare -Wno-unused-but-set-variable -Wno-cast-function-type -Wno-parentheses -Wno-format-overflow -Wno-dangling-else -Wno-maybe-uninitialized)
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -g")
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/engine/inc
    ${CMAKE_SOURCE_DIR}/standard/inc
    ${CMAKE_SOURCE_DIR}/standard/inc/external
    ${CMAKE_SOURCE_DIR}/standard/unibase
    ${FREETYPE_INCLUDE_DIRS}
)

file(GLOB_RECURSE SRC_FILES
    "engine/src/*.cpp"
    "standard/src/*.cpp"
    "standard/unibase/*.c"
    "standard/unibase/*.cpp"
)

if(WIN32)
    list(REMOVE_ITEM SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/standard/src/Compat.cpp")
    add_compile_definitions(NOMINMAX)
    if(MSVC)
        add_compile_options(/FI"${CMAKE_CURRENT_SOURCE_DIR}/standard/inc/Compat.h")
    endif()
endif()

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

set(EXTRA_LIBS "")
if(UNIX)
    list(APPEND EXTRA_LIBS m)
elseif(WIN32)
    list(APPEND EXTRA_LIBS ws2_32 dxguid winmm)
endif()

add_executable(clonk ${SRC_FILES})
target_link_libraries(clonk PRIVATE OpenGL::GL GLEW::GLEW glfw ZLIB::ZLIB ${FREETYPE_LIBRARIES} ${EXTRA_LIBS})
add_dependencies(clonk planet_data_sync)

add_executable(test_loading tests/test_loading.cpp)
target_link_libraries(test_loading PRIVATE ZLIB::ZLIB)
add_dependencies(test_loading planet_data_sync)

add_executable(test_zlib tests/test_zlib.cpp)
target_link_libraries(test_zlib PRIVATE ZLIB::ZLIB)

set(MIDI_LIBS "")
if(UNIX)
    list(APPEND MIDI_LIBS m dl pthread)
endif()

add_executable(clonk_midi launcher/clonk_midi.cpp)
target_link_libraries(clonk_midi PRIVATE ${MIDI_LIBS})
add_dependencies(clonk_midi planet_data_sync)

set(MS_LIBS "")
if(UNIX)
    list(APPEND MS_LIBS pthread)
elseif(WIN32)
    list(APPEND MS_LIBS ws2_32)
endif()

add_executable(masterserver masterserver/masterserver.cpp)
target_link_libraries(masterserver PRIVATE ${MS_LIBS})

# Qt6 Launcher target
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Multimedia REQUIRED)

set(LAUNCHER_SRC_FILES
    launcher/src/main.cpp
    launcher/src/C4Group.cpp
    launcher/src/ConfigManager.cpp
    launcher/src/Utils.cpp
    launcher/src/Win3DWidgets.cpp
    launcher/src/ClonkDialogs.cpp
    launcher/src/OptionsDialog.cpp
    launcher/src/ClonkLauncher.cpp
    launcher/src/SplashWindow.h
)

add_executable(clonk_launcher ${LAUNCHER_SRC_FILES})
target_link_libraries(clonk_launcher PRIVATE Qt6::Widgets Qt6::Multimedia ZLIB::ZLIB)
add_dependencies(clonk_launcher planet_data_sync)

# Copy launcher resource folders to build directory
add_custom_target(launcher_resources_sync ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/launcher/res" "${CMAKE_BINARY_DIR}/res"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/launcher/res_wav" "${CMAKE_BINARY_DIR}/res_wav"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/launcher/res_splash" "${CMAKE_BINARY_DIR}/res_splash"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/launcher/res_dump" "${CMAKE_BINARY_DIR}/res_dump"
    COMMENT "Syncing launcher resources"
)
add_dependencies(clonk_launcher launcher_resources_sync)
