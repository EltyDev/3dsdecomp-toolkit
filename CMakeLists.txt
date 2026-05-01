cmake_minimum_required(VERSION 3.10)

project("3dsdecomp-toolkit" VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

add_executable(${PROJECT_NAME}
    src/parser/function_heuristic.cpp
    src/cli.cpp
    src/parser/arm_parser.cpp
)

FetchContent_Declare(
    capstone
    GIT_REPOSITORY https://github.com/capstone-engine/capstone.git
    GIT_TAG 5.0.7
)

FetchContent_MakeAvailable(capstone)


target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PRIVATE capstone)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)