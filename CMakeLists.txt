cmake_minimum_required(VERSION 2.8)

project(sx127x)

# -g --ggdb3
add_definitions(-O5 -Wall -std=gnu99 -Wno-deprecated-declarations
#add_definitions(-O0 -Wall -std=gnu99 -Wno-deprecated-declarations
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/radio")

file(GLOB COMMON_SRCS "radio/*.c" "radio/linux/*.c" "slrm/*.c" "slrm/linux/*.c")
add_library(sx127x SHARED ${COMMON_SRCS})
target_link_libraries(sx127x pthread m)
install(TARGETS sx127x LIBRARY DESTINATION lib)

add_subdirectory(tests)
