# ==============================================================================
# DIGI-LOAN-RISK-GINI CMAKE BUILD CONFIGURATION
# ==============================================================================
cmake_minimum_required(VERSION 3.28)

project(DigiLoanServer CXX)

# Enforce C++17 Standard (Required by Crow web framework)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add hints for CMake to find Boost
set(BOOST_ROOT "/path/to/boost")
set(Boost_NO_SYSTEM_PATHS ON)

# Set CMP0144 policy to NEW so CMake respects BOOST_ROOT
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

# ==============================================================================
# 1. EXTERNAL SYSTEM DEPENDENCIES
# ==============================================================================
find_package(Threads REQUIRED)
find_package(Boost REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# Optional search for system nlohmann_json package
find_package(nlohmann_json QUIET)

if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
endif()

if(nlohmann_json_FOUND)
    message(STATUS "nlohmann_json found via CMake package configuration.")
endif()

# ==============================================================================
# 2. SOURCE FILES SPECIFICATION
# ==============================================================================
set(SERVER_SOURCES
    src/main.cpp
    src/cibil_score.cpp
    src/database.cpp`
)

# ==============================================================================
# 3. PRIMARY SERVER EXECUTABLE & TARGET CONFIGURATION
# ==============================================================================
add_executable(DigiLoanServer ${SERVER_SOURCES})

# Modern target-based include directories
target_include_directories(DigiLoanServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Link all target libraries
target_link_libraries(DigiLoanServer PRIVATE
    Threads::Threads
    ${Boost_LIBRARIES}
    ${SQLite3_LIBRARIES}
    SQLite::SQLite3
    OpenSSL::SSL
    OpenSSL::Crypto
    ${CURL_LIBRARIES}
)

# Link nlohmann_json target if found as CMake package
if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(DigiLoanServer PRIVATE nlohmann_json::nlohmann_json)
endif()

# ==============================================================================
# 4. STATIC ASSET MANAGEMENT
# Automatically copy frontend assets from public/ to build/public/
# ==============================================================================
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/public/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/public)

# ==============================================================================
# 5. INDEPENDENT CIBIL UNIT TEST EXECUTABLE
# ==============================================================================
add_executable(TestCIBIL 
    tests/test_cibil.cpp 
    src/cibil_score.cpp
    src/database.cpp
)

target_include_directories(TestCIBIL PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# Link both raw library variable AND modern SQLite::SQLite3 target
target_link_libraries(TestCIBIL PRIVATE 
    Threads::Threads
    ${CURL_LIBRARIES}
    ${SQLite3_LIBRARIES}
    SQLite::SQLite3
)

if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(TestCIBIL PRIVATE nlohmann_json::nlohmann_json)
endif()
