cmake_minimum_required (VERSION 3.5)
project (sharpSAT VERSION 1.2 LANGUAGES CXX)

if (WIN32)
    message(FATAL_ERROR "Windows is not supported at this moment")
endif()

include(GNUInstallDirs)

add_library(libsharpSAT STATIC
    src/alt_component_analyzer.cpp
	src/component_analyzer.cpp
	src/component_cache.cpp
	src/component_management.cpp
	src/instance.cpp
	src/new_component_analyzer.cpp
	src/solver.cpp
	src/statistics.cpp
	src/component_types/base_packed_component.cpp
	src/component_types/component_archetype.cpp
)

# Keep the library named as either libsharpSAT.a or sharpSAT.lib
# While having the target's logical name be distinct from sharpSAT (the binary)
set_target_properties(libsharpSAT
    PROPERTIES
      OUTPUT_NAME "sharpSAT"
)

## Add a namespace alias.
# This is useful to abstract over use of the library as installed vs subdirectory build
add_library(sharpSAT::libsharpSAT ALIAS libsharpSAT)

target_compile_features(libsharpSAT
    PUBLIC
      cxx_attributes
      cxx_defaulted_functions
      cxx_deleted_functions
      cxx_final
)

target_include_directories(libsharpSAT
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


find_library(GMP_LIB gmp)
find_library(GMPXX_LIB gmpxx)

target_link_libraries(libsharpSAT PUBLIC ${GMP_LIB} ${GMPXX_LIB})

add_executable(sharpSAT src/main.cpp)
target_link_libraries(sharpSAT libsharpSAT)

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( libsharpSAT PRIVATE -Wall -Wextra )
    target_compile_options( sharpSAT PRIVATE -Wall -Wextra )
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( libsharpSAT PRIVATE /W4 )
    target_compile_options( sharpSAT PRIVATE /W4 )
endif()

# Keep build optimized -- Only for GCC/Clang, MSVC does Debug/Release differently
if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( libsharpSAT PRIVATE -O3 )
    target_compile_options( sharpSAT PRIVATE -O3 )
endif()

# Workaround for libstdc++ + Clang + -std=gnu++11 bug.
set_target_properties(libsharpSAT sharpSAT
    PROPERTIES
      CXX_EXTENSIONS OFF
)



#########
# Tests
##
enable_testing()

# Read all easy instances from a file
file(READ "${PROJECT_SOURCE_DIR}/test/benchmark/easy.txt" SHARPSAT_AUTORUN_TESTS)
string(REGEX REPLACE ";" "\\\\;" SHARPSAT_AUTORUN_TESTS "${SHARPSAT_AUTORUN_TESTS}")
string(REGEX REPLACE "\n" ";" SHARPSAT_AUTORUN_TESTS "${SHARPSAT_AUTORUN_TESTS}")

# Add 1 test for each easy instance
foreach(AUTORUN_TEST ${SHARPSAT_AUTORUN_TESTS})
    add_test(NAME "${AUTORUN_TEST}"
        COMMAND sharpSAT -e @test/benchmark/${AUTORUN_TEST}.txt test/benchmark/${AUTORUN_TEST}.cnf
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
endforeach(AUTORUN_TEST)



###############
# Installation
##
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/sharpSAT)

install(
    TARGETS
      libsharpSAT
    EXPORT sharpSAT-Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    TARGETS
      sharpSAT
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h*")

install(EXPORT sharpSAT-Targets
    FILE sharpSAT-Targets.cmake
    NAMESPACE sharpSAT::
    DESTINATION ${INSTALL_CONFIGDIR}
)


#####################
# ConfigVersion file
##
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/sharpSATConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/CMake/sharpSATConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sharpSATConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

## Install all the helper files
install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/sharpSATConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/sharpSATConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT sharpSAT-Targets FILE ${CMAKE_CURRENT_BINARY_DIR}/sharpSAT-Targets.cmake NAMESPACE sharpSAT::)
export(PACKAGE sharpSAT)
