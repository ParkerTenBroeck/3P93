cmake_minimum_required(VERSION 3.25)

macro(sanitizers target_name)
    if (${ENABLE_ADDRSAN} MATCHES ON)
        target_compile_options(${target_name} PRIVATE -fsanitize=address)
        target_link_options(${target_name} PRIVATE -fsanitize=address)
    endif ()

    if (${ENABLE_UBSAN} MATCHES ON)
        target_compile_options(${target_name} PRIVATE -fsanitize=undefined)
        target_link_options(${target_name} PRIVATE -fsanitize=undefined)
    endif ()

    if (${ENABLE_TSAN} MATCHES ON)
        target_compile_options(${target_name} PRIVATE -fsanitize=thread)
        target_link_options(${target_name} PRIVATE -fsanitize=thread)
    endif ()
endmacro()

macro(compile_options target_name)
    if (NOT ${MOLD} STREQUAL MOLD-NOTFOUND)
        target_compile_options(${target_name} PUBLIC -fuse-ld=mold)
    endif ()

    target_compile_options(${target_name} PRIVATE -flto -fopenmp -Wall -Wextra -Wpedantic -Wno-comment -ffast-math -march=native -mtune=native -std=c++17 -lstdc++)
    target_link_options(${target_name} PRIVATE -flto -fopenmp -Wall -Wextra -Wpedantic -Wno-comment -ffast-math -march=native -mtune=native -std=c++17 -lstdc++)
    sanitizers(${target_name})
endmacro()

project(rasterizer VERSION 0.0.1)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)

include_directories(include/)

file(GLOB_RECURSE PROJECT_BUILD_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

add_executable(rasterizer ${PROJECT_BUILD_FILES}
        src/frame_buffer.h
        src/texture.h)
set_property(TARGET rasterizer PROPERTY CXX_STANDARD 17)

target_include_directories(rasterizer PRIVATE include)
target_link_libraries(${PROJECT_NAME} PUBLIC stdc++fs)
compile_options(rasterizer)
