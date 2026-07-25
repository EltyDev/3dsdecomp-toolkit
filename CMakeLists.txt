cmake_minimum_required(VERSION 3.10)

project("3dsdecomp-toolkit" VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

add_executable(${PROJECT_NAME}
    src/output/asm/asm_block.cpp
    src/output/asm/asm_file.cpp
    src/output/ninja/build_rule.cpp
    src/output/ninja/build_script.cpp
    src/output/ninja/rule.cpp
    src/output/ldscript/ldscript.cpp
    src/output/ldscript/ldsection.cpp
    src/parser/function_heuristic.cpp
    src/parser/arm_parser.cpp
    src/parser/cxi.cpp
    src/utils/atomic_bitmap.cpp
    src/cli.cpp
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