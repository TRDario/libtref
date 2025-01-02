cmake_minimum_required(VERSION 3.23.0...3.30.0 FATAL_ERROR)

project(gtref LANGUAGES C CXX VERSION 1.0.0)

include(FetchContent)
include(cmake/add_embedded_file.cmake)

find_package(tr REQUIRED)

FetchContent_Declare(tinyfd GIT_REPOSITORY http://git.code.sf.net/p/tinyfiledialogs/code)
FetchContent_MakeAvailable(tinyfd)
add_library(tinyfd STATIC ${tinyfd_SOURCE_DIR}/tinyfiledialogs.c)
target_include_directories(tinyfd PUBLIC ${tinyfd_SOURCE_DIR})

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.6)
FetchContent_MakeAvailable(imgui)
add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

add_executable(gtref)
target_compile_features(gtref PRIVATE cxx_std_20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(gtref PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(gtref PRIVATE -fno-trapping-math -fno-math-errno -fno-signed-zeros -fassociative-math -ffp-contract=fast)
    target_compile_options(gtref PRIVATE -march=native)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(gtref PRIVATE /W4 /WX)
endif()
target_link_libraries(gtref PRIVATE tr::tr tref tinyfd imgui)
set_target_properties(gtref PROPERTIES DEBUG_POSTFIX "d")

add_embedded_file(gtref resources/Roboto-Medium.ttf ROBOTO_MEDIUM_TTF)
target_sources(gtref PRIVATE src/main.cpp src/file_edit.cpp src/font.cpp src/help.cpp src/history.cpp src/preview.cpp src/selection.cpp src/view.cpp)

if (TREF_ENABLE_INSTALL)
    install(TARGETS gtref EXPORT gtrefTargets RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()