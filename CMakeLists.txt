cmake_minimum_required(VERSION 3.10)
project(SnakeAI VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Add optimization flags for Release build
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# Source files
set(SOURCES
    main.cpp
    snake.cpp
    nn.cpp
)

# Header files
set(HEADERS
    snake.h
    nn.h
)

# Create executable
add_executable(snake_ai ${SOURCES} ${HEADERS})

# Find and link required libraries
target_link_libraries(snake_ai m)

# Copy configuration file to build directory
configure_file(config.ini ${CMAKE_BINARY_DIR}/config.ini COPYONLY)

# Install target
install(TARGETS snake_ai DESTINATION bin)
install(FILES config.ini DESTINATION share/snake_ai)
