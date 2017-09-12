cmake_minimum_required(VERSION 3.8)
project(desquirr)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        analysis.hpp
        codegen.cpp
        codegen.hpp
        collateexpr.cpp
        collateexpr.hpp
        collatenode.cpp
        collatenode.hpp
        controlflow.cpp
        controlflow.hpp
        dataflow.cpp
        dataflow.hpp
        desquirr.cpp
        desquirr.hpp
        expression.cpp
        expression.hpp
        frontend.cpp
        frontend.hpp
        function.cpp
        function.hpp
        ida-arm.cpp
        ida-arm.hpp
        ida-arm2.hpp
        ida-x86.cpp
        ida-x86.hpp
        idainternal.hpp
        idapro.cpp
        idapro.hpp
        instruction.cpp
        instruction.hpp
        node.cpp
        node.hpp
        usedefine.cpp
        usedefine.hpp
        x86.hpp
        nodes/NodeVisitor.hpp)

add_definitions(-D__MAC__=1)
include_directories(.)
include_directories(../../include)
include_directories(/usr/local/include)

add_executable(desquirr ${SOURCE_FILES})