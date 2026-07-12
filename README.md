Agentic AI - Team Workspace

This is the private team workspace for the **Autonomous Code Generator Agent** project.

Project Overview
An agentic AI system where users describe coding problems in natural language. 
The agent automatically generates C code, compiles it, runs tests, and iteratively fixes errors.

Team Members -

| Role | Name |
Agent Architect       - |  Pradyumna & Sujal |
LLM Prompt Engineer   - |  Pradyumna , Sujal & Deepika |
Compiler & Test Dev   - |  Karthikeya |
Feedback & Report Dev - |  Jashwanth |

Project Structure
src/
├── main.cpp
├── agent
├── compiler
├── tester
└── llm
tests
docs
temp


## Key Features
- Natural language problem input
- Gemini API integration
- GCC compilation via popen()
- Test-driven feedback loop
- Up to 5 agentic iterations
- Final report with diffs
