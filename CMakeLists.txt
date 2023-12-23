cmake_minimum_required(VERSION 3.2)
project(lox)

set(CMAKE_CXX_STANDARD 20)

add_executable( lox

    lox.cpp

    Scanner/Scanner.cpp
    Parser/Parser.cpp
    Interpreter/Interpreter.cpp
    Interpreter/Callables.cpp
    Checker/Checker.cpp
    Error/Error.cpp

    Types/Token.cpp

    tools/printer_ast.cpp
    tools/printer_identifiers.cpp
)

add_executable(generate_ast tools/GenerateAst.cpp)
