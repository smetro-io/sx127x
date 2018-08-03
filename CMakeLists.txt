cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

######################################
# Section : Disable in-source builds #
######################################

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt and CMakeFiles folder." )
endif()

########################################
# Section : Common Build setttings #
########################################


set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O2 -fPIC ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=gnu99 -Wall -O2 -fPIC ${CMAKE_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CUSTOM_COMPILER_FLAGS "-std=gnu99 -Wall -Werror")

################################
# Defines: #
################################
add_definitions(-DUART_DEBUG)

################################
################################
set(PROJECT_TARGET_NAME sx127x)
# Add Target
set(SOURCES
    ${CMAKE_SOURCE_DIR}/radio/sx127x.c
    ${CMAKE_SOURCE_DIR}/radio/sx127x_drv.c
    ${CMAKE_SOURCE_DIR}/radio/sx127x_getset.c
    ${CMAKE_SOURCE_DIR}/radio/sx127x_internal.c
    ${CMAKE_SOURCE_DIR}/radio/linux/gpio.c
    ${CMAKE_SOURCE_DIR}/radio/linux/linux_sx127x_hal.c
    ${CMAKE_SOURCE_DIR}/radio/linux/log.c
    ${CMAKE_SOURCE_DIR}/radio/linux/spi.c
    ${CMAKE_SOURCE_DIR}/radio/linux/timer.c
    ${CMAKE_SOURCE_DIR}/slrm/slrm.c
    ${CMAKE_SOURCE_DIR}/slrm/linux/linux_slrm.c
)

set(HEADERS_TO_INSTALL
    ${CMAKE_SOURCE_DIR}/include/slrm.h
    ${CMAKE_SOURCE_DIR}/include/linux_slrm.h
    ${CMAKE_SOURCE_DIR}/radio/sx127x.h
    ${CMAKE_SOURCE_DIR}/radio/sx127x_hal.h    
)

add_library(${PROJECT_TARGET_NAME} SHARED ${SOURCES})

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS ON)

# Add Target specific includes
target_include_directories(${PROJECT_TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)   
target_include_directories(${PROJECT_TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/radio)

# Add Target specific libraries
TARGET_LINK_LIBRARIES(${PROJECT_TARGET_NAME} -lm -lpthread)

install(TARGETS ${PROJECT_TARGET_NAME} LIBRARY DESTINATION ${TARGET_INSTALL_DIR}/usr/lib)
install(FILES ${HEADERS_TO_INSTALL} DESTINATION ${TARGET_INSTALL_DIR}/usr/include/sx127x)