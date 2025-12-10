cmake_minimum_required(VERSION 3.10)
project(RoveSoPnP_FlowControl CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(RoveFlowControl
    src/parse.cpp
    src/pnp.cpp
    src/tapeLookup.cpp
    src/grbl.cpp
    src/comm.cpp
)

target_include_directories(RoveFlowControl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

########################################################
##               Build standalone runner              ##
########################################################

if (BUILD_STANDALONE)
    # Define executable
    set(EXE_NAME "RoveFlowControlRunner")
    add_executable(${EXE_NAME} src/main.cpp)

    # Link against library
    target_link_libraries(${EXE_NAME} PRIVATE RoveFlowControl)

    target_include_directories(${EXE_NAME} PRIVATE include)

    if (MSVC)
        target_compile_options(${EXE_NAME} PRIVATE /W4 /WX)
    else()
        target_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    message("--      [INFO] Flow control Runner Executable ${EXE_NAME}")
endif()