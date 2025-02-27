cmake_minimum_required(VERSION 3.10)
project(snake_ai)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find SDL2 packages
find_package(SDL2 REQUIRED)

# Include directories
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
)

# Add source files - based on the error messages
set(SOURCES
    main.cpp
    genetic_algorithm.cpp
    renderer.cpp
    neural_network.cpp  # Adding this as it's likely needed
)

# Create executable
add_executable(snake_ai ${SOURCES})

# Link libraries
target_link_libraries(snake_ai
    ${SDL2_LIBRARIES}
    SDL2_ttf  # Direct linking to SDL2_ttf
)

# If the above SDL2_ttf linking doesn't work, try this alternative:
# target_link_libraries(snake_ai
#     ${SDL2_LIBRARIES}
#     -lSDL2_ttf
# )

# Install
install(TARGETS snake_ai DESTINATION bin)
