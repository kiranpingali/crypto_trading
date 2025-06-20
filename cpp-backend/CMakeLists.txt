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

# Add Coinbase API integration component
add_library(coinbase_api STATIC src/coinbase_api.cpp)

# Link dependencies for the Coinbase API component
target_link_libraries(coinbase_api PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)

# Ensure include directory is available for all targets
target_include_directories(coinbase_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link the new component to the main executable
target_link_libraries(stock_server PRIVATE coinbase_api)

# Add Kraken API integration component
add_library(kraken_api STATIC src/kraken_api.cpp)

target_link_libraries(kraken_api PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)
target_include_directories(kraken_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(stock_server PRIVATE kraken_api)

# Add Gemini API integration component
add_library(gemini_api STATIC src/gemini_api.cpp)

target_link_libraries(gemini_api PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)
target_include_directories(gemini_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(stock_server PRIVATE gemini_api) 