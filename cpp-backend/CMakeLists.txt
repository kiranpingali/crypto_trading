cmake_minimum_required(VERSION 3.10)
project(stock_trading_backend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# Add Crow as a header-only library
include(FetchContent)
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow
    GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(Crow)

# Add executable
add_executable(stock_server src/main.cpp)

# Link libraries
target_link_libraries(stock_server PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# Include directories
target_include_directories(stock_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
) 