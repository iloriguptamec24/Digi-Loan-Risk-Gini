# ==============================================================================
# DIGI-LOAN-RISK-GINI CMAKE BUILD CONFIGURATION
# ==============================================================================
cmake_minimum_required(VERSION 3.14)

project(DigiLoanServer CXX)

# Enforce Modern C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# EXTERNAL SYSTEM DEPENDENCIES
# ==============================================================================
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# ==============================================================================
# INCLUDE DIRECTORIES
# ==============================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# ==============================================================================
# SOURCE FILES
# ==============================================================================
set(SOURCES
    src/main.cpp
    src/cibil_score.cpp
    src/database.cpp
)

# ==============================================================================
# TARGET EXECUTABLE & LINK LIBRARIES
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
# STATIC ASSET MANAGEMENT
# Automatically copy public/ assets into build/public/ for Crow static serving
# ==============================================================================
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/public/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/public)