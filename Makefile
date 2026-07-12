# Makefile for Autonomous Code Generator Agent (MinGW / MSYS2 / Linux)

CXX      = g++
CXXFLAGS = -std=gnu++14 -Wall -Wextra -O2 -Isrc
TARGET   = code_agent

SRCS = src/main.cpp \
       src/agent/agent.cpp \
       src/llm/gemini_client.cpp \
       src/compiler/compiler.cpp \
       src/tester/tester.cpp \
       src/report/report.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	-del /Q /F src\main.o src\agent\agent.o src\llm\gemini_client.o src\compiler\compiler.o src\tester\tester.o src\report\report.o 2>nul
	-del /Q /F code_agent.exe 2>nul

.PHONY: all clean
