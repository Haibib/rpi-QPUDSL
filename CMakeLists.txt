cmake_minimum_required(VERSION 3.30)
project(rpi-qpudsl VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CMakeDependentOption)

if (PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif ()

##
# Custom CMake modules

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

##
# Project options

cmake_dependent_option(
    rpidsl_BUILD_TESTING
    "Compile rpi-qpudsl with common compiler warnings"
    "${PROJECT_IS_TOP_LEVEL}"
    BUILD_TESTING
    OFF
)

option(
    rpidsl_ENABLE_WARNINGS
    "Compile rpi-qpudsl with common compiler warnings"
    "${PROJECT_IS_TOP_LEVEL}"
)

option(
    rpidsl_SKIP_INSTALL_RULES
    "Exclude rpi-qpudsl's install rules from the build"
    OFF
)

##
# Process options
if (rpidsl_SKIP_INSTALL_RULES)
    set(CMAKE_SKIP_INSTALL_RULES ON)
endif ()

##
# Dependencies

##

add_library(rpi-qpudsl_library STATIC)
add_library(rpi-qpudsl::library ALIAS rpi-qpudsl_library)

target_compile_features(rpi-qpudsl_library PUBLIC cxx_std_20)

target_sources(
    rpi-qpudsl_library
    PRIVATE
    src/Visitor.cpp
    src/Type.cpp
    src/Printer.cpp
)

target_sources(
    rpi-qpudsl_library
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include
    FILES
    include/Visitor.h
    include/IntrusivePtr.h
    include/IRFwdDecl.h
    include/IRHandle.h
    include/IRNode.h
    include/Error.h
    include/Scope.h
    include/Type.h
    include/Printer.h
)

if (rpi-qpudsl_ENABLE_WARNINGS)
    target_compile_options(
        rpi-qpudsl_library
        PUBLIC
        "$<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:-Wall>"
        "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W3>"
    )
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fno-omit-frame-pointer")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=undefined -fno-omit-frame-pointer")

##
# rpi-qpudsl compiler

add_executable(rpi-qpudsl_compiler compiler.cpp)
add_executable(rpi-qpudsl::compiler ALIAS rpi-qpudsl_compiler)
set_target_properties(
    rpi-qpudsl_compiler
    PROPERTIES
    EXPORT_NAME compiler
    OUTPUT_NAME compiler
)

target_link_libraries(rpi-qpudsl_compiler PRIVATE rpi-qpudsl::library)

##
# Testing

# if (rpi-qpudsl_BUILD_TESTING)
#     add_subdirectory(tests)
# endif ()

##
# Installation rules

if (NOT CMAKE_SKIP_INSTALL_RULES)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(rpi-qpudsl_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/cmake/rpi-qpudsl"
        CACHE STRING "Install destination for rpi-qpudsl CMake package files")

    install(
        TARGETS rpi-qpudsl_compiler
        EXPORT rpi-qpudsl-targets
        COMPONENT rpi-qpudsl-runtime
    )

    install(
        EXPORT rpi-qpudsl-targets
        NAMESPACE rpi-qpudsl::
        DESTINATION "${rpi-qpudsl_INSTALL_CMAKEDIR}"
        COMPONENT rpi-qpudsl-development
    )

    configure_package_config_file(
        cmake/rpi-qpudsl-config.cmake.in
        cmake/rpi-qpudsl-config.cmake
        INSTALL_DESTINATION "${rpi-qpudsl_INSTALL_CMAKEDIR}"
    )

    write_basic_package_version_file(
        cmake/rpi-qpudsl-config-version.cmake
        COMPATIBILITY SameMajorVersion
    )

    install(
        DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmake/"
        DESTINATION "${rpi-qpudsl_INSTALL_CMAKEDIR}"
        COMPONENT rpi-qpudsl-development
    )
endif ()