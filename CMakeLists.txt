cmake_minimum_required(VERSION 3.10)
project(RoveSoPnP_FlowControl CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(RoveSoPnP_FlowControl
    main.cpp
    parse.cpp
    pnp.cpp
    tapeLookup.cpp
    grbl.cpp
    comm.cpp
)
