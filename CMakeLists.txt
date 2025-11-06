cmake_minimum_required(VERSION 3.10)

project(cppLua VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE
    Engine/LuaLex.cpp
    Engine/LuaParser.cpp
    Engine/LuaVM.cpp)

add_library(cpplua STATIC ${SOURCE})
target_include_directories(cpplua 
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine  # 绝对路径，避免歧义
)

add_executable(helloworld Example/helloworld.cpp)
target_link_libraries(helloworld PRIVATE cpplua)
target_include_directories(helloworld PRIVATE Engine)
