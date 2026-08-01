# Digi-Loan-Risk-Gini
Traditional retail lending decision workflows often rely on static, monolithic rule setups or manual underwriting processes that introduce latency, high operational overhead, and human bias

📖 Developer FAQ & Operations Guide (FAQ.md)
Welcome to the Digi-Loan-Risk-Gini setup and operations manual. This document answers common questions regarding building, executing, and troubleshooting the C++ Crow web application.

🛠️ Build & Compilation
Q1: How do I build the application from scratch?
Run CMake to generate build configuration files, followed by cmake --build to compile the C++ source code:

Bash
# 1. Create and navigate into the build directory
mkdir -p build && cd build

# 2. Generate build files
cmake ..

# 3. Compile the source code
cmake --build .

# 4. How do I perform a clean rebuild?
Remove build artifacts and recompile <br>
If you modified CMakeLists.txt, added new dependencies, or updated static assets in public/index.html, perform a clean build: <br>
Bash <br>
rm -rf build/ <br>
mkdir build && cd build <br>
cmake .. <br>
cmake --build . <br>

# 5. Why do I need to rebuild if I only modified public/index.html? <br>
The CMakeLists.txt file uses file(COPY ...) to transfer static frontend assets from public/ into the output executable folder (build/public). Running cmake --build build copies the latest version of index.html into the target location so Crow can serve it.

# 6. 🚀 Running the Application
How do I run the application executable?<br>
Always execute the binary from the project root directory (where public/ and src/ are located) to ensure proper relative path resolution for static assets and database files: <br>
Run from the root directory: <br>
Bash<br>
./build/DigiLoanServer<br>

# 7. Is there a single-line command to compile and run directly?
Yes, run this command from the project root directory:<br>

Bash<br>
cmake --build build && ./build/DigiLoanServer<br>

# 8. Where can I access the Web UI?
Once the server output prints 🚀 DIGI-LOAN-RISK-GINI RUNNING AT http://localhost:18080, open your web browser and navigate to:<br>

Local Machine: http://localhost:18080<br>

GitHub Codespaces: Open the Ports tab in VS Code / Codespaces and open Port 18080 in your browser.<br>

# 9. 🔌 Port Management & Process Termination
What do I do if port 18080 is already in use?<br>
If you see an error like bind: Address already in use or Failed to listen on port 18080, an instance of DigiLoanServer (or another <br>process) is running in the background.<br>

Step 1: Identify the Process ID (PID)<br>
Bash<br>
lsof -i :18080<br>
OR<br>
netstat -nlp | grep 18080<br>
Step 2: Kill the Process<br>
Bash<br>
Kill process using PID (replace <PID> with the actual process number)<br>
kill -9 <PID><br>

OR Kill the process directly by port number in one step:<br>
fuser -k 18080/tcp<br>
Step 3: Kill by Process Name<br>
Bash<br>
killall DigiLoanServer<br>

🐛 Troubleshooting Common Errors<br>
# 8. bash: ./build/DigiLoanServer: No such file or directory
Cause: You ran cmake .. to configure the project, but have not yet compiled the executable file.<br>
Solution: Run the build step before executing:<br>

Bash<br>
cd build && cmake --build . && cd ..<br>
./build/DigiLoanServer<br>

# 9. CMake Error: Could NOT find asio (missing: ASIO_INCLUDE_DIR)
Cause: Crow requires the standalone Asio header library for asynchronous networking.<br>
Solution: Install the developer headers on Debian/Ubuntu/Codespaces:<br>

Bash<br>
sudo apt-get update && sudo apt-get install -y libasio-dev<br>

# 10.  error: operands to '?:' have different types 'crow::json::detail::r_string' and 'const char [N]'
Cause: Implicit type conversion failure between Crow's custom JSON string wrapper and a C-style string literal.<br>
Solution: Explicitly cast Crow string values to std::string:<br>

C++<br>
std::string loanType = "Personal";<br>
if (body.has("loanType")) {<br>
    loanType = std::string(body["loanType"].s());<br>
}<br>

# 11. Where is the SQLite database stored?
The application programmatically creates a data/ directory at the project root and stores the SQLite database in data/digi_loan_risk.db.<br>

To reset or clear all application data, stop the server and delete the database file:<br>

Bash<br>
rm -rf data/digi_loan_risk.db<br>