
cmake_minimum_required(VERSION 3.21)

project(speex_project)

enable_language(CXX C ASM)

set(LIB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/speex_prj/include/speex ${CMAKE_CURRENT_SOURCE_DIR}/PLC)
set(LIB_SPEX ${CMAKE_CURRENT_SOURCE_DIR}/speex_prj/lib/libspeex.so)

add_executable(target main.c ${CMAKE_CURRENT_SOURCE_DIR}/PLC/lowcfe.c)

target_link_libraries(target ${LIB_SPEX})
target_link_libraries(target m)
target_include_directories(target PUBLIC ${LIB_INCLUDE})

# include(ExternalProject)

# ExternalProject_Add(speex_project
#     SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/speex
#     CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/speex/configure --prefix=/home/vitalii/Speex_project/Speex_project/build_prj --enable-static
#     BUILD_COMMAND ${MAKE})

