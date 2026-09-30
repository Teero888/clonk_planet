cmake_minimum_required(VERSION 3.30)
project(ClonkPlanet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenGL REQUIRED)
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

# copy resources to build directory

add_custom_target(planet_data_sync ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/planet_data" "${CMAKE_BINARY_DIR}/"
    COMMENT "Syncing game resources"
)

set(EXTRA_LIBS "")
if(UNIX)
    list(APPEND EXTRA_LIBS m)
elseif(WIN32)
    list(APPEND EXTRA_LIBS ws2_32 dxguid winmm)
endif()

add_executable(clonk ${SRC_FILES})
target_link_libraries(clonk PRIVATE OpenGL::GL GLEW::GLEW glfw ZLIB::ZLIB ${FREETYPE_LIBRARIES} ${EXTRA_LIBS})
add_dependencies(clonk planet_data_sync)

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

file(GLOB LAUNCHER_SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/launcher/src/*.cpp
    ${CMAKE_SOURCE_DIR}/launcher/src/*.h
)

add_executable(clonk_launcher ${LAUNCHER_SRC_FILES} standard/src/StdIniRegistry.cpp)

target_link_libraries(clonk_launcher PRIVATE Qt6::Widgets Qt6::Multimedia ZLIB::ZLIB)
add_dependencies(clonk_launcher planet_data_sync)

# Copy launcher resource folders to build directory
add_custom_target(launcher_resources_sync ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/launcher/data" "${CMAKE_BINARY_DIR}/data"
    COMMENT "Syncing launcher resources"
)
add_dependencies(clonk_launcher launcher_resources_sync)

# Tests (ctest --test-dir build)
enable_testing()
add_executable(test_groupedit tests/test_groupedit.cpp launcher/src/GroupEdit.cpp launcher/src/C4Group.cpp)
target_link_libraries(test_groupedit PRIVATE Qt6::Core ZLIB::ZLIB)
add_test(NAME groupedit COMMAND test_groupedit "${CMAKE_SOURCE_DIR}/planet_data")

add_executable(test_explorer tests/test_explorer.cpp launcher/src/ExplorerModel.cpp launcher/src/GroupEdit.cpp
    launcher/src/C4Group.cpp launcher/src/C4TextDoc.cpp launcher/src/LauncherRes.cpp launcher/src/Utils.cpp)
target_link_libraries(test_explorer PRIVATE Qt6::Widgets Qt6::Multimedia ZLIB::ZLIB)
add_test(NAME explorer COMMAND test_explorer "${CMAKE_SOURCE_DIR}/planet_data" "${CMAKE_SOURCE_DIR}/launcher/data")
set_tests_properties(explorer PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(test_config tests/test_config.cpp launcher/src/ConfigManager.cpp standard/src/StdIniRegistry.cpp)
add_test(NAME config COMMAND test_config)
