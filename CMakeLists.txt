cmake_minimum_required(VERSION 3.16)
project(CraneSimulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# Duong dan freeglut tren may (co the doi bang -DFREEGLUT_ROOT=...)
set(FREEGLUT_ROOT "D:/freeglut-3.4.0" CACHE PATH "Thu muc source freeglut")

set(FREEGLUT_TARGET "")

if(EXISTS "${FREEGLUT_ROOT}/CMakeLists.txt")
    message(STATUS "Build freeglut tu: ${FREEGLUT_ROOT}")
    set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
    set(FREEGLUT_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
    set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory("${FREEGLUT_ROOT}" "${CMAKE_BINARY_DIR}/freeglut")
    set(FREEGLUT_TARGET freeglut_static)
else()
    find_path(GLUT_INCLUDE_DIR GL/freeglut.h)
    find_library(GLUT_LIBRARY NAMES freeglut freeglut_static glut)
    if(NOT GLUT_INCLUDE_DIR OR NOT GLUT_LIBRARY)
        message(FATAL_ERROR
            "Khong tim thay freeglut.\n"
            "Dat FREEGLUT_ROOT tro den thu muc source freeglut, hoac cai vcpkg:\n"
            "  vcpkg install freeglut:x64-windows")
    endif()
endif()

add_executable(crane_sim
    src/main.cpp
    src/crane.cpp
    src/texture.cpp
    src/geometry.cpp
    src/asset_path.cpp
)

target_include_directories(crane_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(FREEGLUT_TARGET)
    target_link_libraries(crane_sim PRIVATE
        OpenGL::GL
        ${FREEGLUT_TARGET}
    )
    target_include_directories(crane_sim PRIVATE "${FREEGLUT_ROOT}/include")
else()
    target_include_directories(crane_sim PRIVATE ${GLUT_INCLUDE_DIR})
    target_link_libraries(crane_sim PRIVATE
        OpenGL::GL
        ${GLUT_LIBRARY}
    )
endif()

if(WIN32)
    target_link_libraries(crane_sim PRIVATE glu32 winmm)
endif()

add_custom_command(TARGET crane_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:crane_sim>/assets
)
