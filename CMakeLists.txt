cmake_minimum_required (VERSION 2.8)
project (SHARPSAT) 

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 -Wall")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -O3 -DNDEBUG -Wall")

set(CMAKE_CXX_FLAGS_PROFILING "${CMAKE_CXX_FLAGS_PROFILING} -std=c++11 -O3 -g -DNDEBUG -Wall")

IF(UNIX)
  IF(APPLE)
    include_directories(/opt/local/include)
  ELSE(APPLE)
    
    
  ENDIF(APPLE)
ELSE(UNIX)

ENDIF(UNIX)


find_library(GMP_LIB gmp)
find_library(GMPXX_LIB gmpxx)


add_subdirectory(src)

add_executable (sharpSAT src/main.cpp)
target_link_libraries (sharpSAT libsharpSAT ${GMP_LIB} ${GMPXX_LIB}) 
