#pragma once
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "coinbase_api.h"
#include "kraken_api.h"
#include "gemini_api.h"

class OrderBook {
public:
    OrderBook(CoinbaseAPI* coinbase, KrakenAPI* kraken, GeminiAPI* gemini);
    ~OrderBook();

    // Fetch and build the consolidated order book for the top 10 pairs
    nlohmann::json buildConsolidatedOrderBook();

private:
    CoinbaseAPI* coinbase_;
    KrakenAPI* kraken_;
    GeminiAPI* gemini_;

    // Helper to get top 10 pairs by volume (static for now, can be dynamic)
    std::vector<std::string> getTopPairs() const;

    // Fetch order book from each exchange
    nlohmann::json fetchCoinbaseOrderBook(const std::string& pair);
    nlohmann::json fetchKrakenOrderBook(const std::string& pair);
    nlohmann::json fetchGeminiOrderBook(const std::string& pair);

    // Merge order books
    nlohmann::json mergeOrderBooks(const std::string& pair, const std::vector<nlohmann::json>& books);
}; 