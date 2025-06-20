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

## Consolidated Order Book

This project includes a C++ component that builds a consolidated order book for the top 10 crypto pairs by volume, aggregating data from Coinbase, Kraken, and Gemini.

- **Location:** `cpp-backend/include/order_book.h`, `cpp-backend/src/order_book.cpp`
- **Features:**
  - Fetches order book data from all three exchanges for the top 10 pairs
  - Aggregates and merges bids/asks into a single consolidated order book per pair
  - Uses nlohmann::json for all data representation
  - Designed for extensibility and further analytics
- **Build:** Integrated via CMake; linked to the main executable

You can use the `OrderBook` class to fetch and build a consolidated order book. See the header file for available methods and required parameters.

## Web-based Front-End for Consolidated Order Book

This project includes a web-based front-end to view the consolidated order book for the top 10 crypto pairs by volume.

- **Location:** `public/orderbook.html`, `public/orderbook.js`
- **How it works:**
  - The Node.js backend exposes an API endpoint `/api/orderbook` that runs the C++ backend to fetch the consolidated order book as JSON.
  - The front-end fetches this data and displays it in a responsive table for each crypto pair.

### How to Run

1. **Build the C++ backend:**
   - Make sure you have built the C++ backend so that `./cpp-backend/build/stock_server` exists and supports the `--orderbook` flag to output the consolidated order book as JSON.

2. **Install Node.js dependencies:**
   ```sh
   npm install
   ```

3. **Start the Node.js server:**
   ```sh
   node server.js
   ```

4. **View the order book:**
   - Open your browser and go to [http://localhost:3000/orderbook.html](http://localhost:3000/orderbook.html)

You will see a table for each of the top 10 crypto pairs, showing the consolidated bids and asks from Coinbase, Kraken, and Gemini.

## Crypto Trading Application

This project now includes a trading application that allows you to:
- View the consolidated order book for the top 10 crypto pairs (from Coinbase, Kraken, and Gemini)
- Enter a trade (pair, buy/sell, quantity)
- The backend finds the best price across all three exchanges and executes the trade on the best venue
- The result of the trade (including which exchange was used) is displayed to the user

### How to Use

1. **Start the backend and frontend as described above.**
2. **Open your browser and go to:**
   [http://localhost:3000/trade.html](http://localhost:3000/trade.html)
3. **Enter your trade details and submit.**
4. **The app will show the best price, the exchange used, and the execution result.**

> **Note:** You must configure your real API keys/secrets for Coinbase, Kraken, and Gemini in the backend for live trading.
