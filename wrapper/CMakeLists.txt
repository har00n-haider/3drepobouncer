# Required for exporting the wrapper API in the built library
add_definitions( -DREPO_WRAPPER_API_LIBRARY)

add_subdirectory(src)

include_directories(src ../bouncer/src )

add_library(3drepobouncerwrapper SHARED ${WRAPPER_SOURCES})

target_link_libraries(3drepobouncerwrapper 3drepobouncer)

install(DIRECTORY src/ DESTINATION include FILES_MATCHING PATTERN "*.h")

install(TARGETS 3drepobouncerwrapper DESTINATION lib)
