cmake_minimum_required (VERSION 2.8)

project(server)

set(CMAKE_CXX_STANDARD 11)

include_directories(/usr/include/hiredis)

find_library(HIREDIS_LIBRARY hiredis)

add_executable(server servermain.cpp file.cpp user.cpp sign.cpp friend.cpp group.cpp transfer.cpp)

target_link_libraries(server ${HIREDIS_LIBRARY})