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

if(WIN32 AND NOT MINGW)
    message(FATAL_ERROR "Windows builds use MinGW-w64 (MSYS2 UCRT64): the engine relies on its POSIX functions")
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
    list(APPEND EXTRA_LIBS ws2_32)
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
if(MINGW)
    # Windows 7 API level: newer headers emit inline wrappers for Windows 10 only functions
    target_compile_definitions(clonk PRIVATE _WIN32_WINNT=0x0601)
    target_compile_definitions(clonk_midi PRIVATE _WIN32_WINNT=0x0601)
endif()
add_dependencies(clonk_midi planet_data_sync)

set(MS_LIBS "")
if(UNIX)
    list(APPEND MS_LIBS pthread)
elseif(WIN32)
    list(APPEND MS_LIBS ws2_32)
endif()

add_executable(masterserver masterserver/masterserver.cpp)
target_link_libraries(masterserver PRIVATE ${MS_LIBS})

option(CLONK_BUILD_LAUNCHER "Build the Qt launcher and its tests" ON)
if(CLONK_BUILD_LAUNCHER)

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

endif() # CLONK_BUILD_LAUNCHER

# ------------------------------------------------------------------------------------------------
# Installation: a portable game folder (cmake --install build --prefix <dir>), used for releases.
# Everything is placed next to the executables like in the original installation; the launcher
# and the engine write players, savegames and scenarios into that folder.

set(CLONK_PROGRAMS clonk clonk_midi masterserver)
if(CLONK_BUILD_LAUNCHER)
    list(APPEND CLONK_PROGRAMS clonk_launcher)
endif()

if(UNIX AND NOT APPLE)
    # bundled libraries in lib/; DT_RPATH (not RUNPATH) so that it also applies to their dependencies
    set_target_properties(${CLONK_PROGRAMS} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
    foreach(program IN LISTS CLONK_PROGRAMS)
        target_link_options(${program} PRIVATE "LINKER:--disable-new-dtags")
    endforeach()
endif()

install(TARGETS ${CLONK_PROGRAMS} RUNTIME DESTINATION .)
install(DIRECTORY planet_data/ DESTINATION .)
install(FILES LICENSE.txt README.md DESTINATION .)
install(FILES engine/license.txt DESTINATION . RENAME License-Engine.txt)
if(CLONK_BUILD_LAUNCHER)
    install(DIRECTORY launcher/data/ DESTINATION data)
endif()

# Make the folder self-contained: Qt plugins and the libraries that are not part of the system
if(WIN32 OR (UNIX AND NOT APPLE))
    set(CLONK_QT_PLUGIN_DIR "")
    set(CLONK_QT_PLUGINS "")
    if(CLONK_BUILD_LAUNCHER)
        set(CLONK_QT_PLUGIN_DIR "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_PLUGINS}")
    endif()
    if(WIN32)
        set(CLONK_DEPLOY_WINDOWS TRUE)
        set(CLONK_QT_PLUGINS "platforms/qwindows.dll" "multimedia/*.dll")
        # the MinGW / MSYS2 directories with the DLLs (in order of preference)
        get_filename_component(compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
        set(dll_dirs "${compiler_dir}")
        foreach(root IN LISTS CMAKE_FIND_ROOT_PATH)
            list(APPEND dll_dirs "${root}/bin")
        endforeach()
        set(CLONK_DLL_DIRS "${dll_dirs}" CACHE STRING "Directories with the DLLs to ship, in order of preference")
        set(CLONK_DEPLOY_SEARCH_DIRS "${CLONK_DLL_DIRS}")
        set(CLONK_DEPLOY_SYSTEM_REGEXES ".*[/\\\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\\\].*")
    else()
        set(CLONK_DEPLOY_WINDOWS FALSE)
        set(CLONK_QT_PLUGINS "platforms/libqxcb.so" "platforms/libqwayland*.so" "platforms/libqoffscreen.so" "platforminputcontexts/*.so"
            "xcbglintegrations/*.so" "wayland-shell-integration/*.so" "wayland-decoration-client/*.so"
            "wayland-graphics-integration-client/*.so")
        set(CLONK_DEPLOY_SEARCH_DIRS "")
        if(CLONK_BUILD_LAUNCHER)
            set(CLONK_DEPLOY_SEARCH_DIRS "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_LIBS}")
        endif()
        # Core libraries of every Linux desktop are left to the system
        set(system_libs
            ld-linux libc[.]so libm[.]so libmvec libdl[.]so libpthread[.]so librt[.]so libresolv[.]so
            libutil[.]so libgcc_s[.]so libstdc[+][+][.]so libatomic[.]so
            libGL[.]so libGLX libEGL libGLdispatch libOpenGL libGLU libdrm libgbm libvulkan
            libX11 libxcb libXau libXdmcp libXext libXrender libXi[.]so libXrandr libXcursor libXinerama
            libXfixes libXxf86vm libxkbcommon libwayland libSM[.]so libICE[.]so
            libasound libpulse libpipewire libjack
            libfontconfig libfreetype libexpat libz[.]so libbz2 liblzma libzstd libbrotli libpng libharfbuzz
            libgraphite2 libglib-2 libgobject-2 libgio-2 libgmodule-2 libgthread-2 libpcre2-8 libffi
            libdbus-1 libsystemd libudev libcap[.]so libselinux libmount libblkid libuuid libgssapi libkrb5
            libk5crypto libcom_err libssl libcrypto libgcrypt libgpg-error liblz4)
        list(TRANSFORM system_libs PREPEND ".*/")
        set(CLONK_DEPLOY_SYSTEM_REGEXES "${system_libs}")
    endif()
    configure_file(cmake/DeployRuntime.cmake.in DeployRuntime.cmake @ONLY)
    install(SCRIPT "${CMAKE_BINARY_DIR}/DeployRuntime.cmake")
endif()
