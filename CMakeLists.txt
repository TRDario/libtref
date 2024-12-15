cmake_minimum_required(VERSION 3.23.0...3.30.0 FATAL_ERROR)

project(tref LANGUAGES CXX VERSION 1.0.0)

option(TREF_ENABLE_INSTALL "whether to enable the install rule" ON)
option(TREF_BUILD_TOOLS "whether to build tools for working with tref files" ON)

find_package(lz4 REQUIRED)

add_library(tref STATIC src/tref.cpp)
target_sources(tref PUBLIC FILE_SET HEADERS BASE_DIRS include FILES include/tref/qoi.h include/tref/tref.hpp)
target_compile_features(tref PUBLIC cxx_std_20)
target_link_libraries(tref PUBLIC lz4)

if(TREF_ENABLE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS tref
        EXPORT trefTargets
        FILE_SET HEADERS
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(
        EXPORT trefTargets
        NAMESPACE tref::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tref"
    )

    configure_package_config_file(
        cmake/trefConfig.cmake.in
        "${PROJECT_BINARY_DIR}/trefConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tref"
    )
    install(
        FILES "${PROJECT_BINARY_DIR}/trefConfig.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tref"
    )

    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/trefConfigVersion.cmake"
        COMPATIBILITY SameMinorVersion)
    install(
        FILES "${PROJECT_BINARY_DIR}/trefConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tref"
    )
endif ()

if (TREF_BUILD_TOOLS)
	add_subdirectory(tools/trefc)
endif ()