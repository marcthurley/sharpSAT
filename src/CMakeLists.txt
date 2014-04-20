add_subdirectory (component_types)

SET(SOURCES alt_component_analyzer.cpp 
	component_analyzer.cpp
	component_cache.cpp
	component_management.cpp
	instance.cpp
	new_component_analyzer.cpp
	solver.cpp
	statistics.cpp)


ADD_LIBRARY(libsharpSAT ${SOURCES} component_types)
target_link_libraries (libsharpSAT component_types) 
