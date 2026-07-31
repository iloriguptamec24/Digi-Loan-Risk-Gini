cmake_minimum_required(VERSION 3.14)
project(DigiLoanRiskGini CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate system dependencies
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Fetch third-party header libraries directly from GitHub
include(FetchContent)

# 1. Crow Framework
FetchContent_Declare(
  Crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG master
)

# 2. jwt-cpp Framework
FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG master
)

FetchContent_MakeAvailable(Crow jwt-cpp)

# Include Header Directories
include_directories(include ${jwt-cpp_SOURCE_DIR}/include)

# Executable Configuration
add_executable(DigiLoanServer src/main.cpp src/database.cpp)
target_link_libraries(DigiLoanServer PRIVATE SQLite3::SQLite3 Crow::Crow OpenSSL::SSL OpenSSL::Crypto)
