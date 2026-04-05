cmake_minimum_required(VERSION 3.10)
project(RoveSoPnP_FlowControl CXX)
set(CMAKE_CXX_STANDARD 20)

# Enable Qt's Meta-Object Compiler
set(CMAKE_AUTOMOC ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core SerialPort)

# CRITICAL FIX: Explicitly include the headers in the source list so AUTOMOC finds Q_OBJECT
file(GLOB_RECURSE LIB_SRC CONFIGURE_DEPENDS "src/*.cpp" "include/*.hpp" "include/*.h")
add_library(RoveFlowControl ${LIB_SRC})

target_include_directories(RoveFlowControl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the Qt libraries to your flow control library
target_link_libraries(RoveFlowControl PUBLIC Qt6::Core Qt6::SerialPort)

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