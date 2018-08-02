cmake_minimum_required(VERSION 2.8)

project(sx127x)

# -g --ggdb3
add_definitions(-O5 -Wall -std=gnu99 -Wno-deprecated-declarations -Wno-unused-value
#add_definitions(-O0 -Wall -std=gnu99 -Wno-deprecated-declarations
)
add_definitions(-DUART_DEBUG)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/radio")

file(GLOB COMMON_SRCS "radio/*.c" "radio/linux/*.c" "slrm/*.c" "slrm/linux/*.c")
file(GLOB HEADERS "include/linux_slrm.h" "include/slrm.h" "radio/sx127x.h" "radio/sx127x_hal.h")
add_library(sx127x SHARED ${COMMON_SRCS})
target_link_libraries(sx127x pthread m)
install(TARGETS sx127x LIBRARY DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/sx127x)
