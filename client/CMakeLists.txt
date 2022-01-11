add_subdirectory(src)

include_directories(src ../bouncer/src )
add_executable(3drepobouncerClient ${CLIENT_SOURCES}) 
target_link_libraries(3drepobouncerClient 3drepobouncer)


install(TARGETS 3drepobouncerClient DESTINATION bin)
