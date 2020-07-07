cmake_minimum_required(VERSION 2.8)

project(360SCVPDemo)

AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(../../ DIR_SRC2)

ADD_DEFINITIONS("-z noexecstack -z relro -z now -fstack-protector-strong -fPIE -fPIC -pie -O0 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-S -Wall -Werror -g -c -fPIC -std=c++11")

INCLUDE_DIRECTORIES(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(.)

ADD_EXECUTABLE(360SCVPDemo ${DIR_SRC} ${DIR_SRC2})

#target_link_libraries(360SCVPDemo 360SCVP)

if(NOT DEFINED CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX STREQUAL "")
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "..." FORCE)
endif()
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

if(NOT CMAKE_INSTALL_BINDIR)
    set(CMAKE_INSTALL_BINDIR "bin")
endif()

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib")
endif()

if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR "include")
endif()

