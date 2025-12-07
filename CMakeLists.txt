cmake_minimum_required(VERSION 3.10)
project(RoveSoPnP_FlowControl CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(RoveSoPnP_FlowControl
    src/main.cpp
    src/parse.cpp
    src/pnp.cpp
    src/tapeLookup.cpp
    src/grbl.cpp
    src/comm.cpp
)

target_include_directories(RoveSoPnP_FlowControl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)