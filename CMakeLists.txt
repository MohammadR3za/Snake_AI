
cmake_minimum_required(VERSION 3.10)
project(SnakeAI VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/game_state.cpp
    src/food.cpp
    src/snake.cpp
    src/game.cpp
    src/neural_network.cpp
    src/snake_agent.cpp
    src/genetic_algorithm.cpp
    src/game_environment.cpp
    src/training.cpp
    src/training_visualizer.cpp
    src/play_mode.cpp
)

# Header files (for IDE integration)
set(HEADERS
    include/direction.h
    include/game_state.h
    include/food.h
    include/snake.h
    include/game.h
    include/neural_network.h
    include/snake_agent.h
    include/genetic_algorithm.h
    include/game_environment.h
    include/training.h
    include/training_visualizer.h
    include/play_mode.h
)

# Create executable
add_executable(snake_ai ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(snake_ai ${CURSES_LIBRARIES} Threads::Threads)

# Install target
install(TARGETS snake_ai DESTINATION bin)

# Optional: Create a directory structure if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(snake_ai PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(snake_ai PRIVATE /W4)
endif()

# Add debug symbols in Debug mode
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
# Optimize in Release mode
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
