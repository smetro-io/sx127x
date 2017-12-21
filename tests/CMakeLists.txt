include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(linux-sx127x "linux-sx127x.c")
target_link_libraries(linux-sx127x m sx127x)
install(TARGETS linux-sx127x DESTINATION bin)
