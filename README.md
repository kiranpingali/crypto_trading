# crypto_trading
An institutional-grade crypto trading platform that will serve to provide best execution and prices for crypto traders.

## Getting Started

Instructions on how to get started with the project will go here.

## Installation

Installation instructions will go here.

## Usage

Usage instructions will go here.

## Contributing

Guidelines for contributing to the project will go here.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Coinbase API Integration

This project includes a C++ component for connecting to the Coinbase crypto exchange and executing orders via the Coinbase Pro API.

- **Location:** `cpp-backend/include/coinbase_api.h`, `cpp-backend/src/coinbase_api.cpp`
- **Features:**
  - Authenticated connection to Coinbase Pro
  - Market order placement (buy/sell)
  - Uses CURL for HTTP requests and nlohmann::json for JSON handling
  - HMAC SHA256 request signing (OpenSSL)
- **Build:** Integrated via CMake; linked to the main executable

You can use the `CoinbaseAPI` class to place orders on Coinbase Pro. See the header file for available methods and required parameters.

## Kraken API Integration

This project includes a C++ component for connecting to the Kraken crypto exchange and executing orders via the Kraken API.

- **Location:** `cpp-backend/include/kraken_api.h`, `cpp-backend/src/kraken_api.cpp`
- **Features:**
  - Authenticated connection to Kraken
  - Market order placement (buy/sell)
  - Uses CURL for HTTP requests and nlohmann::json for JSON handling
  - HMAC SHA512 request signing (OpenSSL)
- **Build:** Integrated via CMake; linked to the main executable

You can use the `KrakenAPI` class to place orders on Kraken. See the header file for available methods and required parameters.

## Gemini API Integration

This project includes a C++ component for connecting to the Gemini crypto exchange and executing orders via the Gemini API.

- **Location:** `cpp-backend/include/gemini_api.h`, `cpp-backend/src/gemini_api.cpp`
- **Features:**
  - Authenticated connection to Gemini
  - Market order placement (buy/sell)
  - Uses CURL for HTTP requests and nlohmann::json for JSON handling
  - HMAC SHA384 request signing (OpenSSL)
- **Build:** Integrated via CMake; linked to the main executable

You can use the `GeminiAPI` class to place orders on Gemini. See the header file for available methods and required parameters.
