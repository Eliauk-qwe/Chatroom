
cmake_minimum_required(VERSION 3.10)
project(Chatroom)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 查找 nlohmann/json 库
find_package(nlohmann_json REQUIRED)

# 包含项目头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}
    /usr/include/hiredis  # hiredis 头文件路径
)

# 查找 hiredis 库（服务器依赖）
find_library(HIREDIS_LIBRARY hiredis)
if(NOT HIREDIS_LIBRARY)
    message(FATAL_ERROR "hiredis library not found")
endif()

# 服务器源文件（根据实际文件调整）
set(SERVER_SOURCES
server/servermain.cpp
server/file.cpp
server/friend_quit.cpp
server/friend.cpp
server/new_friend.cpp
server/access_group.cpp
server/group_apply.cpp
server/group.cpp
server/signup.cpp
server/user.cpp
)

# 客户端源文件（根据实际文件调整）
set(CLIENT_SOURCES
client/clientmain.cpp
client/file.cpp
client/friend_quit.cpp
client/friend.cpp
client/new_friend.cpp
client/access_group.cpp
client/group_apply.cpp
client/group.cpp
client/signup.cpp
client/user.cpp
)

# 添加服务器可执行文件
add_executable(server ${SERVER_SOURCES})
target_link_libraries(server
    ${HIREDIS_LIBRARY}
    nlohmann_json::nlohmann_json  # 链接 json 库
    pthread
)

# 添加客户端可执行文件
add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client
    nlohmann_json::nlohmann_json  # 链接 json 库
    pthread
)