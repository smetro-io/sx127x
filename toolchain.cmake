set(CMAKE_SYSTEM_NAME Linux)
set(TARGET_SYSROOT_DIR ${CMAKE_SOURCE_DIR}/../../../)

set(CMAKE_SYSROOT ${TARGET_SYSROOT_DIR}/output/rootfs /usr/arm-linux-gnueabihf)

set(CMAKE_C_COMPILER /usr/bin/arm-linux-gnueabihf-gcc-4.7)
set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-g++-4.7)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

include_directories(SYSTEM /usr/arm-linux-gnueabihf/include/c++/4.7.4)
include_directories(SYSTEM /usr/arm-linux-gnueabihf/include/c++/4.7.4/arm-linux-gnueabihf)

link_directories(/usr/arm-linux-gnueabihf/lib/)
link_directories(${TARGET_SYSROOT_DIR}/output/rootfs/lib)
link_directories(${TARGET_SYSROOT_DIR}/output/rootfs/lib/arm-linux-gnueabihf)
link_directories(${TARGET_SYSROOT_DIR}/output/rootfs/usr/lib)
link_directories(${TARGET_SYSROOT_DIR}/output/rootfs/usr/lib/arm-linux-gnueabihf)

set(TARGET_INSTALL_DIR ${TARGET_SYSROOT_DIR}/output/rootfs)
