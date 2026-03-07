cmake_minimum_required(VERSION 3.25)
project(fake-keyboard VERSION 0.1.0 LANGUAGES CXX)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};-warnings-as-errors=*")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17.0")
        message(FATAL_ERROR "Clang 17+ required for C++23 support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
        message(FATAL_ERROR "GCC 13+ required for C++23 support")
    endif()
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(BLUEZ REQUIRED bluez)
pkg_check_modules(DBUS REQUIRED dbus-1)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/build/Release/generators")

find_package(spdlog REQUIRED)
find_package(nlohmann_json QUIET)

add_library(fakekbd
    src/libfakekbd/bluetooth/adapter.cpp
    src/libfakekbd/bluetooth/dbus.cpp
    src/libfakekbd/bluetooth/l2cap.cpp
    src/libfakekbd/bluetooth/sdp.cpp
    src/libfakekbd/config.cpp
    src/libfakekbd/hid/keyboard.cpp
    src/libfakekbd/hid/report.cpp
)

target_include_directories(fakekbd
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${BLUEZ_INCLUDE_DIRS}
        ${DBUS_INCLUDE_DIRS}
)

target_link_libraries(fakekbd
    PUBLIC
        spdlog::spdlog
    PRIVATE
        ${BLUEZ_LIBRARIES}
        ${DBUS_LIBRARIES}
)

if(nlohmann_json_FOUND)
    target_link_libraries(fakekbd PUBLIC nlohmann_json::nlohmann_json)
endif()

target_compile_options(fakekbd PRIVATE
    -Wall -Wextra -Wpedantic
)

add_executable(fake-keyboard
    src/main.cpp
)

target_link_libraries(fake-keyboard
    PRIVATE
        fakekbd
)

enable_testing()

add_executable(fake-keyboard-tests
    tests/test_main.cpp
    tests/test_config.cpp
    tests/test_report.cpp
    tests/test_sdp.cpp
)

target_link_libraries(fake-keyboard-tests
    PRIVATE
        fakekbd
)

find_package(Catch2 REQUIRED)
target_link_libraries(fake-keyboard-tests
    PRIVATE
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(fake-keyboard-tests)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS fakekbd
    EXPORT fakekbdTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY src/libfakekbd/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libfakekbd
    FILES_MATCHING PATTERN "*.hpp"
)

install(TARGETS fake-keyboard
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT fakekbdTargets
    FILE fakekbdTargets.cmake
    NAMESPACE fakekbd::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fakekbd
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fakekbdConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/fakekbdConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fakekbd
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/fakekbdConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/fakekbdConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/fakekbdConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fakekbd
)
