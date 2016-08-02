cmake_minimum_required(VERSION 3.5)
project(cs148_project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    common/controls.cpp
    common/controls.h
    common/mesh.cpp
    common/mesh.h
    common/model.cpp
    common/model.h
    common/setup.cpp
    common/setup.h
    Camera.h
    lightstate.cpp
    lightstate.h
    main.cpp
    Makefile
    Readme.md
    scene.cpp
    scene.h
    shader.frag
    Shader.h
    shader.vs)

add_executable(pyramid ${SOURCE_FILES})