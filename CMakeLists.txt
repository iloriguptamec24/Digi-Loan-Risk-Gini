cmake_minimum_required(VERSION 3.14)
project(DigiLoanRiskGini CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find System Dependencies
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Disable boost dependency for Crow and force nlohmann-json for jwt-cpp
set(CROW_USE_BOOST OFF CACHE BOOL "" FORCE)
set(JWT_JSON_TRAITS_ENGINE "nlohmann-json" CACHE STRING "" FORCE)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JWT_BUILD_TESTS OFF CACHE BOOL "" FORCE)

include(FetchContent)

# 1. Fetch standalone Asio
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG master
)
FetchContent_MakeAvailable(asio)

# Set Asio include path for Crow's CMake finder
set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "" FORCE)
include_directories(${ASIO_INCLUDE_DIR})
add_definitions(-DASIO_STANDALONE)

# 2. Fetch Crow Framework
FetchContent_Declare(
  Crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG master
)

# 3. Fetch jwt-cpp Framework
FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG master
)

FetchContent_MakeAvailable(Crow jwt-cpp)

# Include Header Directories
include_directories(include ${jwt-cpp_SOURCE_DIR}/include)

# Executable Target
add_executable(DigiLoanServer 
    src/main.cpp 
    src/database.cpp 
    src/cibil_score.cpp
)

# Copy static frontend assets into build folder
file(COPY ${CMAKE_SOURCE_DIR}/public DESTINATION ${CMAKE_BINARY_DIR})

# SQLite3 Target Fallback
if(NOT TARGET SQLite3::SQLite3)
  if(TARGET SQLite::SQLite3)
    add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
  else()
    add_library(SQLite3::SQLite3 UNKNOWN IMPORTED)
    set_target_properties(SQLite3::SQLite3 PROPERTIES
      IMPORTED_LOCATION "${SQLite3_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIRS}"
    )
  endif()
endif()

# Link Libraries
target_link_libraries(DigiLoanServer PRIVATE 
    SQLite3::SQLite3 
    Crow::Crow 
    OpenSSL::SSL 
    OpenSSL::Crypto
    jwt-cpp::jwt-cpp
)