
#================Source and header files===============
add_subdirectory(src)

set(VERSION_MAJOR 4)
set(VERSION_MINOR 19_2)

include_directories( ${CRYPTOLENS_INCLUDE_DIR} src ../)
add_definitions(-DREPO_API_LIBRARY)

add_library(3drepobouncer SHARED ${SOURCES})
set_target_properties(3drepobouncer PROPERTIES RELEASE_POSTFIX "_${VERSION_MAJOR}_${VERSION_MINOR}")
set_target_properties(3drepobouncer PROPERTIES DEBUG_POSTFIX "_${VERSION_MAJOR}_${VERSION_MINOR}_d")

target_link_libraries(3drepobouncer ${CRYPTOLENS_LIBRARIES})

install(DIRECTORY src/ DESTINATION include FILES_MATCHING PATTERN "*.h")
install(TARGETS 3drepobouncer DESTINATION lib)

