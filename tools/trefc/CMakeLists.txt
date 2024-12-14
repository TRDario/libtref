cmake_minimum_required(VERSION 3.23.0...3.30.0 FATAL_ERROR)

if (WIN32)
    option(TREFC_ANSI_COLORS "Enable the use of ANSI escape sequences for colored text output" OFF)
else ()
    option(TREFC_ANSI_COLORS "Enable the use of ANSI escape sequences for colored text output" ON)
endif ()

project(trefc LANGUAGES C CXX VERSION 1.0.0)
add_executable(trefc src/input.cpp src/image.cpp src/output.cpp src/main.cpp)
target_compile_features(trefc PRIVATE cxx_std_20)
target_link_libraries(trefc PRIVATE tref)

if (TREFC_ANSI_COLORS)
    target_compile_definitions(trefc PRIVATE TREFC_ANSI_COLORS)
endif ()