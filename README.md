# Autonomous Code Generator Agent

An AI agent that converts natural language problem descriptions into working C code — automatically generating, compiling, testing, and fixing code through an agentic feedback loop.

## How It Works

1. **User Input** — Describe a coding problem in plain English
2. **Code Generation** — Gemini LLM generates a complete C program
3. **Compilation** — GCC compiles the generated code
4. **Testing** — Agent runs test cases and checks output
5. **Auto-Fix** — If tests fail, the agent sends errors back to the LLM and retries (up to 5 iterations)
6. **Report** — Generates a final report with diffs showing code improvements

## Project Structure

```
src/
├── main.cpp              # Entry point
├── agent/                # Core agent logic (orchestrates the loop)
├── llm/                  # Gemini API client
├── compiler/             # GCC compilation wrapper
├── tester/               # Test runner and output verification
├── report/               # Report generation with diffs
└── utils/                # JSON parser, env loader
tests/
└── test_cases.json       # Sample test cases
```

## Key Features

- Natural language problem input
- Gemini API integration for code generation
- GCC compilation with error capture
- Test-driven feedback loop with automatic retries
- Up to 5 agentic iterations
- Final report with before/after diffs

## Getting Started

### Prerequisites

- C++ compiler (GCC / MinGW / MSYS2)
- `curl` (for API calls)
- Gemini API key ([Get one here](https://makersuite.google.com/app/apikey))

### Setup

```bash
# Clone the repo
git clone https://github.com/Sujal-Sharma-93/Autonomous-Code-Generator-Agent-Public.git
cd Autonomous-Code-Generator-Agent-Public

# Create your .env file
cp .env.example .env
# Edit .env and add your Gemini API key

# Build
make

# Run
./code_agent
```

### Usage

```
========== Autonomous Code Generator Agent ==========
Describe the coding problem:
> Write a program to reverse a string
```

## Team

| Role                  | Members                    |
|-----------------------|----------------------------|
| Agent Architect       | Pradyumna & Sujal          |
| LLM Prompt Engineer   | Pradyumna, Sujal & Deepika |
| Compiler & Test Dev   | Karthikeya                 |
| Feedback & Report Dev | Jashwanth                  |

## Tech Stack

- **Language:** C++ (C++17)
- **LLM:** Google Gemini API
- **Build:** Make / CMake
- **Testing:** Custom test framework
