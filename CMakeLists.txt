# ==============================================================================
# DIGI-LOAN-RISK-GINI CMAKE BUILD CONFIGURATION
# ==============================================================================
cmake_minimum_required(VERSION 3.14)

project(DigiLoanServer CXX)

# Enforce C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 1. EXTERNAL SYSTEM DEPENDENCIES
# ==============================================================================
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# ==============================================================================
# 2. INCLUDE DIRECTORIES
# ==============================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# ==============================================================================
# 3. SOURCE FILES SPECIFICATION
# ==============================================================================
set(SOURCES
    src/main.cpp
    src/cibil_score.cpp
    src/database.cpp
)

# ==============================================================================
# 4. PRIMARY SERVER EXECUTABLE & LINK LIBRARIES
# ==============================================================================
add_executable(DigiLoanServer ${SOURCES})

target_link_libraries(DigiLoanServer PRIVATE
    Threads::Threads
    ${SQLite3_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    ${CURL_LIBRARIES}
)

# ==============================================================================
# 5. STATIC ASSET MANAGEMENT
# Automatically copy frontend assets from public/ to build/public/
# ==============================================================================
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/public/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/public)

# ==============================================================================
# 6. INDEPENDENT CIBIL UNIT TEST EXECUTABLE
# ==============================================================================
add_executable(TestCIBIL 
    tests/test_cibil.cpp 
    src/cibil_score.cpp
)

target_link_libraries(TestCIBIL PRIVATE 
    ${CURL_LIBRARIES}
)