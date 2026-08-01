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
Why libasio-dev is required: The Crow C++ web framework relies on standalone Asio header files for
asynchronous I/O and HTTP networking.
```

## 🏗️ 2. Build & Compilation Instructions
#### 🚀 Standard Build Procedure
Run CMake to generate build configuration files, followed by cmake --build to compile the C++ source code:
* **Bash**
* Create and navigate into the build directory : **mkdir -p build && cd build**
* Generate build configuration : **cmake ..**
* Compile source code into the binary executable
    **cmake --build .**
* Go to main dir : **cd ..**
* Run Application : **./build/DigiLoanServer**

#### 🚀 Clean Rebuild
```bash
If you modified CMakeLists.txt, updated static assets in public/index.html, or want a fresh start, perform a clean rebuild:
    Bash
    Remove build cache and recompile
       -rf build/
        cmake -B build
        cmake --build build

[!IMPORTANT]
    Static Assets Copy: CMake uses file(COPY ...) to sync public/index.html into build/public/.
    Running cmake --build build ensures any frontend updates are copied to the output target directory so Crow can serve them.
```
## 🚀 3. Running the Application
```bash
Always execute the binary from the project root directory (where public/ and src/ are located) to
ensure proper relative path resolution for static assets and database files:
    Bash
    Run from the project root directory:
        ./build/DigiLoanServer
    ⚡ One-Liner (Build & Run) : To compile and launch immediately in a single step from the project root:
        Bash
        cmake --build build && ./build/DigiLoanServer
```
## 🌐 4. Accessing the Web UI
```bash
Once launched, the console will confirm server startup:

    =======================================================
    🚀 DIGI-LOAN-RISK-GINI RUNNING AT http://localhost:18080
    =======================================================
    Open your web browser and navigate to:
        Local Machine: http://localhost:18080 or http://localhost:<port number>
```

## 🔌 5. Port Management & Process Termination
If you see an error like bind: Address already in use or Failed to listen on port 18080, an instance of DigiLoanServer (or another process) 
is already running in the background.
```bash
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
```

## 🐛 6. Developer FAQ & Troubleshooting
```bash
Q1: Why do I need to rebuild if I only modified public/index.html?
    The CMakeLists.txt build configuration transfers static frontend assets from public/ into the output executable
    folder (build/public/). 
    Running cmake --build build copies the latest version of index.html into the target location so Crow can serve it.

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
```
