cmake_minimum_required(VERSION 3.10)
project(RoveSoPnP_FlowControl CXX)
set(CMAKE_CXX_STANDARD 20)

# 1. Gather Source Files (Exclude main.cpp for the library)
file(GLOB_RECURSE LIB_SRC CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER LIB_SRC EXCLUDE REGEX "src/main.cpp$")

add_library(RoveFlowControl ${LIB_SRC})

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