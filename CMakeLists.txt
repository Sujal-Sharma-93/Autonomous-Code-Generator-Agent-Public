cmake_minimum_required(VERSION 3.14)
project(AutonomousCodeGenerator LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(code_agent
    src/main.cpp
    src/agent/agent.cpp
    src/llm/gemini_client.cpp
    src/compiler/compiler.cpp
    src/tester/tester.cpp
    src/report/report.cpp
)

target_include_directories(code_agent PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/src
)
