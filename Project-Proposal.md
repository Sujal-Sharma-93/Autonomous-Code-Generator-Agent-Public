1.] Project Proposal: AI Code Generator Agent

2.] Project Overview :-
An agentic AI system that automatically generates C code from natural language descriptions, compiles it, runs tests, and iteratively fixes errors until all tests pass.

3.] Problem Statement :-
Writing code manually for every programming problem is time-consuming. This project creates an intelligent agent that can understand natural language problems and generate correct C code through an automated feedback loop.

4.] Proposed Solution :-
A C++ agentic pipeline that implements the ReAct (Reason + Act) pattern:

1. Reason: - Parse the problem description
2. Act Call - Gemini API to generate C code
3. Observe - Compile with GCC and run tests
4. Repeat -  Feed errors back to Gemini for fixes
5. Output - Generate final report with diffs and results

5.] Team Members & Roles :-

Agent Architect -  Sujal Sharma 
LLM Prompt Engineer -  [Teammate 1] 
Compiler & Test Dev - [Teammate 2] 
Feedback & Report Dev - [Teammate 3] 
Support - [Teammate 4] 

6.] Technology Stack :-

| Component | Technology |
|-----------|------------|
| Language | C++ |
| AI Model | Google Gemini Flash API |
| Compiler | GCC (via popen()) |
| Version Control | GitHub |

7.] Key Features :-

- Natural language problem input
- Agentic ReAct loop (Reason → Act → Observe)
- Automatic C code generation
- Compilation error detection and fixing
- Test-driven feedback
- Iterative fixing (up to 5 attempts)
- Final report with diff visualization

8.]Project Timeline :-

| Week | Tasks |
|------|-------|
| Week 1 | Project setup, API integration, basic agent loop |
| Week 2 | Compiler integration, test framework |
| Week 3 | Error feedback system, diff generation |
| Week 4 | Testing, debugging, final report |

9.] Deliverables :-

1.  Project Proposal (this document)
2.  Agent Loop Flowchart
3.  Mid-Review Demo
4.  Final Working Code
5.  GitHub Repository with Documentation

## Expected Outcome
A fully functional agent that can generate correct C code for simple to medium complexity problems within 5 iterations, with full visibility into the generation process.
