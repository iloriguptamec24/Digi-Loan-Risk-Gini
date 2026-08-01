# ==============================================================================
# 1. CMAKE VERSION & PROJECT SETUP
# ==============================================================================
# Minimum required CMake version for target features and dependency finders
cmake_minimum_required(VERSION 3.14)

# Project Name and programming language specification (CXX = C++)
project(DigiLoanServer CXX)

# Enforce Modern C++17 Standard across all compiled source files
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 2. EXTERNAL DEPENDENCIES & LIBRARIES
# ==============================================================================
# Find system POSIX threads package required for multi-threaded web server
find_package(Threads REQUIRED)

# Find SQLite3 development libraries for relational database storage
find_package(SQLite3 REQUIRED)

# Find OpenSSL libraries required by Crow HTTP server for SSL/crypto functionality
find_package(OpenSSL REQUIRED)

# Find libcurl development package for outbound REST API HTTP POST calls (CIBIL API)
find_package(CURL REQUIRED)

# ==============================================================================
# 3. INCLUDE DIRECTORIES
# ==============================================================================
# Include custom header files from include/ along with external library header paths
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# ==============================================================================
# 4. SOURCE FILE SPECIFICATION
# ==============================================================================
# Define C++ implementation files to be compiled into the final executable
set(SOURCES
    src/main.cpp
    src/cibil_score.cpp
    src/database.cpp
)

# ==============================================================================
# 5. TARGET EXECUTABLE & LINK LIBRARIES
# ==============================================================================
# Define the final executable binary name and its associated source list
add_executable(DigiLoanServer ${SOURCES})

# Link internal threads, SQLite3, OpenSSL, and libcurl to the executable
target_link_libraries(DigiLoanServer PRIVATE
    Threads::Threads
    ${SQLite3_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    ${CURL_LIBRARIES}
)

# ==============================================================================
# 6. STATIC ASSET MANAGEMENT
# ==============================================================================
# Automatically copy frontend assets from public/ into build/public/ during build.
# This enables Crow to resolve relative paths and serve index.html seamlessly.
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/public/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/public)
