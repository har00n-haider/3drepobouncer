add_definitions(-DREPO_WRAPPER_API_LIBRARY)	

set(WRAPPER_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/c_sharp_wrapper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/repo_csharp_interface.cpp
	CACHE STRING "WRAPPER_SOURCES" FORCE)

set(TOOLS_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/src/c_sharp_wrapper.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/repo_csharp_interface.h
	CACHE STRING "WRAPPER_HEADERS" FORCE)

include_directories(src ../bouncer/src 
	${Boost_INCLUDE_DIR} ${MONGO_INCLUDE_DIR} ${ASSIMP_INCLUDE_DIR} ${OCCT_INCLUDE_DIR} ${IFCOPENSHELL_INCLUDE_DIR})
add_library(3drepobouncerwrapper SHARED ${WRAPPER_SOURCES})
target_link_libraries(3drepobouncerwrapper 3drepobouncer
	${Boost_LIBRARIES} ${MONGO_LIBRARIES} ${ASSIMP_LIBRARIES} ${OCCT_LIBRARIES} ${IFCOPENSHELL_GEOMLIB} ${IFCOPENSHELL_PARSERLIB})

install(DIRECTORY src/ DESTINATION include FILES_MATCHING PATTERN "*.h")
install(TARGETS 3drepobouncerwrapper DESTINATION lib)
