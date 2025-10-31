cmake_minimum_required(VERSION 3.25)

include(FetchContent)

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

    unset(NIX_ENFORCE_NO_NATIVE)

    target_compile_options(${target_name} PRIVATE -flto -Wall -Wextra -Wpedantic -Wno-comment -ffast-math -std=c++17)
    target_link_options(${target_name} PRIVATE  -flto -Wall -Wextra -Wpedantic -Wno-comment -ffast-math -std=c++17 -lstdc++)

    target_compile_options(${target_name} PRIVATE -march=native -mtune=native)
    target_link_options(${target_name} PRIVATE -march=native -mtune=native)

    sanitizers(${target_name})
endmacro()

project(rasterizer VERSION 0.0.1)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)
option(ENABLE_OPENGL "Enable OpenGL" OFF)
option(ENABLE_OPENMP "Enable OpenMP" ON)
option(ENABLE_OPENMP "Enable OpenMPI" OFF)

if (ENABLE_OPENGL)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(glfw3
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            # Release 3.4
            GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579
            FIND_PACKAGE_ARGS)
    FetchContent_MakeAvailable(glfw3)

    find_package(OpenGL REQUIRED)
endif()

include_directories(include/)
include_directories(src/)

file(GLOB_RECURSE PROJECT_BUILD_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

add_executable(rasterizer ${PROJECT_BUILD_FILES})
set_property(TARGET rasterizer PROPERTY CXX_STANDARD 17)

target_include_directories(rasterizer PRIVATE include)
target_link_libraries(${PROJECT_NAME} PUBLIC stdc++fs)

compile_options(rasterizer)
if (ENABLE_OPENGL)
    target_compile_options(${PROJECT_NAME} PRIVATE -DGUI)
    target_link_options(${PROJECT_NAME} PRIVATE)

    target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL::GL)
endif()

if (ENABLE_OPENMP)
    target_compile_options(${PROJECT_NAME} PRIVATE -DUSE_OPEN_MP -fopenmp)
    target_link_options(${PROJECT_NAME} PRIVATE -fopenmp)
endif()


if (ENABLE_OPENMP)
    target_compile_options(${PROJECT_NAME} PRIVATE -DUSE_OPEN_MPI -fopenmp)
    target_link_options(${PROJECT_NAME} PRIVATE -fopenmp)
endif()