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
        node.cpp
        node.hpp
        usedefine.cpp
        usedefine.hpp
        x86.hpp
        nodes/NodeListVisitor.cpp
        nodes/NodeListVisitor.h
        instructions/instruction.cpp
        instructions/instruction.hpp
        instructions/label.hpp
        instructions/case.hpp
        instructions/break.hpp
        instructions/continue.hpp
        instructions/throw.hpp
        instructions/unary/if.hpp
        instructions/unary/unaryinstruction.hpp
        instructions/unary/push.hpp
        instructions/unary/pop.hpp
        instructions/unary/jump.hpp
        instructions/unary/return.hpp
        instructions/unary/dowhile.hpp
        instructions/unary/while.hpp
        instructions/unary/switch.hpp
        instructions/binary/binary.hpp
        instructions/binary/conditionaljump.hpp
        instructions/binary/assignment.hpp
)

add_definitions(-D__MAC__=1)
include_directories(.)
include_directories(../../include)
include_directories(./instructions)
include_directories(./instructions/unary)
include_directories(./instructions/binary)
include_directories(/usr/local/include)

add_executable(desquirr ${SOURCE_FILES})