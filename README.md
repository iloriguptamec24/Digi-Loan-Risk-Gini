Markdown
# 🛠️ Build, Run & Developer Operations Guide

Welcome to the complete operational manual for the **Digi-Loan-Risk-Gini** C++ Crow microservice. This document covers prerequisites, build procedures, execution steps, port management, and common troubleshooting FAQs.

---

## 📋 System Prerequisites

Ensure your build environment satisfies the following requirements:

* **Operating System:** Linux (Ubuntu 20.04+, Debian 11+, or GitHub Codespaces)
* **C++ Compiler:** `g++` or `clang` supporting **C++17** or higher
* **Build System:** `CMake` (v3.14+)
* **Build Tools:** `make` or `ninja`

---

## 📦 1. Install System Dependencies

Before compiling, install the required development headers (`libasio-dev`, `SQLite3`, and `OpenSSL`) using your package manager:

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    cmake \
    libasio-dev \
    libsqlite3-dev \
    libssl-dev
[!NOTE]
Why libasio-dev is required: The Crow C++ web framework relies on standalone Asio header files for asynchronous I/O and HTTP networking.
```
## 🏗️ 2. Build & Compilation Instructions
Standard Build Procedure
Run CMake to generate build configuration files, followed by cmake --build to compile the C++ source code:
* Bash

* #### a. Create and navigate into the build directory
mkdir -p build && cd build

#### b. Generate build configuration
cmake ..

#### c. Compile source code into the binary executable
cmake --build .
Clean Rebuild
If you modified CMakeLists.txt, updated static assets in public/index.html, or want a fresh start, perform a clean rebuild:

Bash
#### Remove build cache and recompile
rm -rf build/
cmake -B build
cmake --build build
[!IMPORTANT]
Static Assets Copy: CMake uses file(COPY ...) to sync public/index.html into build/public/. Running cmake --build build ensures any frontend updates are copied to the output target directory so Crow can serve them.

🚀 3. Running the Application
Always execute the binary from the project root directory (where public/ and src/ are located) to ensure proper relative path resolution for static assets and database files:

Bash
# Run from the project root directory:
./build/DigiLoanServer
⚡ One-Liner (Build & Run)
To compile and launch immediately in a single step from the project root:

Bash
cmake --build build && ./build/DigiLoanServer
🌐 4. Accessing the Web UI
Once launched, the console will confirm server startup:

Plaintext
=======================================================
🚀 DIGI-LOAN-RISK-GINI RUNNING AT http://localhost:18080
=======================================================
Open your web browser and navigate to:

Local Machine: http://localhost:18080

GitHub Codespaces: Open the Ports tab in VS Code / Codespaces and click on Port 18080.

🔌 5. Port Management & Process Termination
If you see an error like bind: Address already in use or Failed to listen on port 18080, an instance of DigiLoanServer (or another process) is already running in the background.

Option A: Kill Directly by Port (Recommended)
Bash
fuser -k 18080/tcp
Option B: Identify and Kill by PID
Bash
# 1. Find process ID running on port 18080
lsof -i :18080

# 2. Kill process using PID (replace <PID> with the actual process number)
kill -9 <PID>
Option C: Kill by Process Name
Bash
killall DigiLoanServer
🐛 6. Developer FAQ & Troubleshooting
Q1: Why do I need to rebuild if I only modified public/index.html?
The CMakeLists.txt build configuration transfers static frontend assets from public/ into the output executable folder (build/public/). Running cmake --build build copies the latest version of index.html into the target location so Crow can serve it.

Q2: bash: ./build/DigiLoanServer: No such file or directory
[!NOTE]
Cause: You ran cmake .. to configure the build environment, but have not yet compiled the executable file.

Solution: Run the build step before executing:

Bash
cd build && cmake --build . && cd ..
./build/DigiLoanServer
Q3: CMake Error: Could NOT find asio (missing: ASIO_INCLUDE_DIR)
[!IMPORTANT]
Cause: Crow requires the standalone Asio header library for asynchronous networking.

Solution: Install the developer headers on Debian / Ubuntu / GitHub Codespaces:

Bash
sudo apt-get update && sudo apt-get install -y libasio-dev
Q4: error: operands to '?:' have different types 'crow::json::detail::r_string' and 'const char [N]'
[!NOTE]
Cause: Implicit type conversion failure between Crow's custom JSON string wrapper (r_string) and a standard C-style string literal (const char*).

Solution: Explicitly cast Crow string values to std::string:

C++
std::string loanType = "Personal";
if (body.has("loanType")) {
    loanType = std::string(body["loanType"].s());
}
Q5: Where is the SQLite database stored, and how do I reset it?
The application programmatically creates a data/ directory at the project root and stores the SQLite database in data/digi_loan_risk.db.

To reset or clear all application data, stop the server and delete the database file:

Bash
rm -f data/digi_loan_risk.db
[!TIP]
Deleting digi_loan_risk.db is completely safe. The C++ server will automatically recreate the database schema upon its next launch.


1. Headers
Markdown
# Heading Level 1
## Heading Level 2
### Heading Level 3
#### Heading Level 4
2. Text Formatting
Markdown
*Italic text* or _Italic text_
**Bold text** or __Bold text__
***Bold and italic***
~~Strikethrough~~
3. Lists
Bulleted (Unordered)
Markdown
* Item 1
* Item 2
  * Sub-item 2.1
  * Sub-item 2.2
Numbered (Ordered)
Markdown
1. First step
2. Second step
3. Third step
4. Code & Syntax Highlighting
Inline Code
Markdown
Use the `cmake --build build` command to compile.
Code Blocks
To create a code block, wrap your code in triple backticks (```) on their own lines. You can specify a language after the opening backticks for syntax highlighting:

Markdown
```bash
# Terminal command
cmake -B build
cmake --build build
```

```cpp
// C++ code block
#include <iostream>

int main() {
    std::cout << "Hello World!\n";
    return 0;
}
```
5. Links and Images
Markdown
[Clickable Link Text](https://www.example.com)

![Image Alt Text](https://via.placeholder.com/150)
6. Blockquotes & GitHub Callouts
Standard Quote
Markdown
> This is a blockquote line.
GitHub-Flavored Callout Boxes
Markdown
> [!NOTE]
> Useful information that users should know.

> [!TIP]
> Helpful advice for doing things better or faster.

> [!IMPORTANT]
> Key information users need to know to achieve their goal.

> [!WARNING]
> Urgent info that needs immediate user attention to avoid problems.
7. Tables
Markdown
| Feature | Supported | Description |
| :--- | :---: | ---: |
| **Left Aligned** | **Center Aligned** | **Right Aligned** |
| Feature A | Yes | Standard mode |
| Feature B | No | Experimental |
8. Horizontal Rule & Line Breaks
Markdown
---
To create a line break within a single paragraph without adding extra spacing, end a line with two spaces or use <br>:

Markdown
First line  
Second line directly below

First line<br>
Second line directly below