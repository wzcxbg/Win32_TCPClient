cmake_minimum_required(VERSION 3.10)

project(Win32TCPClient)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCE_FILES
    src/main.cpp
    src/window.cpp
    src/tcp_client.cpp
)

# Add header files
set(HEADER_FILES
    include/window.h
    include/tcp_client.h
    include/resource.h
)

# Create executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Link against Windows Sockets library
target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)

# Set Windows subsystem
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)