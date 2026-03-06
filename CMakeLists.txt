cmake_minimum_required(VERSION 4.2)
project(fake-keyboard VERSION 0.1.0 LANGUAGES CXX)

# C++23 required
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler check
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17.0")
        message(FATAL_ERROR "Clang 17+ required for C++23 support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
        message(FATAL_ERROR "GCC 13+ required for C++23 support")
    endif()
endif()

# Find system packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(BLUEZ REQUIRED bluez)

# Main executable
add_executable(fake-keyboard
    src/main.cpp
)

target_link_libraries(fake-keyboard
    PRIVATE
        ${BLUEZ_LIBRARIES}
)

target_include_directories(fake-keyboard
    PRIVATE
        ${BLUEZ_INCLUDE_DIRS}
)

target_compile_options(fake-keyboard PRIVATE
    -Wall -Wextra -Wpedantic
)

# Install
install(TARGETS fake-keyboard RUNTIME DESTINATION bin)
