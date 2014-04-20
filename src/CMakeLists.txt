cmake_minimum_required (VERSION 2.8)
project (SHARPSAT) 

add_compile_options(-std=c++11)

IF(UNIX)
  IF(APPLE)
    include_directories(/opt/local/include)
  ELSE(APPLE)
    
    
  ENDIF(APPLE)
ELSE(UNIX)

ENDIF(UNIX)

add_subdirectory (component_types)

include_directories(${SHARPSAT_SOURCE_DIR}/component_types)
link_directories (${SHARPSAT_BINARY_DIR}/component_types) 

find_library(GMP_LIB gmp)
find_library(GMPXX_LIB gmpxx)


SET(SOURCES alt_component_analyzer 
	component_analyzer
	component_cache
	component_management
	instance
	new_component_analyzer
	solver
	statistics)

FOREACH(source ${SOURCES})
  ADD_LIBRARY(${source} ${source}.cpp)
ENDFOREACH(source)



add_executable (sharpSAT main.cpp)
target_link_libraries (sharpSAT ${SOURCES} ${GMP_LIB} ${GMPXX_LIB} component_types) 