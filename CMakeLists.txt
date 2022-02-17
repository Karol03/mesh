cmake_minimum_required(VERSION 3.5)

project(mesh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(mesh
    main.cpp

    mesh.hpp
    meshbuilder.hpp

    objects/edge.hpp
    objects/imobject.hpp
    objects/node.hpp
    objects/types.hpp

    utils/meshpack.hpp
)
