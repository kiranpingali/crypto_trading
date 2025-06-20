#include "order_book.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <algorithm>

OrderBook::OrderBook(CoinbaseAPI* coinbase, KrakenAPI* kraken, GeminiAPI* gemini)
    : coinbase_(coinbase), kraken_(kraken), gemini_(gemini) {}

OrderBook::~OrderBook() {}

std::vector<std::string> OrderBook::getTopPairs() const {
    // Static list for demonstration; in production, fetch dynamically by volume
    return {"BTC-USD", "ETH-USD", "USDT-USD", "SOL-USD", "XRP-USD", "DOGE-USD", "ADA-USD", "AVAX-USD", "LINK-USD", "MATIC-USD"};
}

nlohmann::json OrderBook::fetchCoinbaseOrderBook(const std::string& pair) {
    // Coinbase uses dashes, e.g., BTC-USD
    std::string endpoint = "/products/" + pair + "/book?level=2";
    return coinbase_->sendRequest("GET", endpoint);
}

nlohmann::json OrderBook::fetchKrakenOrderBook(const std::string& pair) {
    // Kraken uses XBTUSD, ETHUSD, etc. Map as needed
    std::string kraken_pair = pair;
    if (kraken_pair == "BTC-USD") kraken_pair = "XBTUSD";
    else kraken_pair.erase(std::remove(kraken_pair.begin(), kraken_pair.end(), '-'), kraken_pair.end());
    nlohmann::json req = {{"pair", kraken_pair}};
    return kraken_->sendRequest("/0/public/Depth", req);
}

nlohmann::json OrderBook::fetchGeminiOrderBook(const std::string& pair) {
    // Gemini uses lowercase, no dash, e.g., btcusd
    std::string gemini_pair = pair;
    std::transform(gemini_pair.begin(), gemini_pair.end(), gemini_pair.begin(), ::tolower);
    gemini_pair.erase(std::remove(gemini_pair.begin(), gemini_pair.end(), '-'), gemini_pair.end());
    std::string endpoint = "/v1/book/" + gemini_pair;
    return gemini_->sendRequest(endpoint, nullptr);
}

nlohmann::json OrderBook::mergeOrderBooks(const std::string& pair, const std::vector<nlohmann::json>& books) {
    // Merge bids and asks from all books, sort by price
    std::vector<std::vector<double>> bids, asks;
    for (const auto& book : books) {
        if (book.contains("bids")) {
            for (const auto& bid : book["bids"]) {
                double price = std::stod(bid[0].get<std::string>());
                double size = std::stod(bid[1].get<std::string>());
                bids.push_back({price, size});
            }
        }
        if (book.contains("asks")) {
            for (const auto& ask : book["asks"]) {
                double price = std::stod(ask[0].get<std::string>());
                double size = std::stod(ask[1].get<std::string>());
                asks.push_back({price, size});
            }
        }
    }
    std::sort(bids.begin(), bids.end(), [](const auto& a, const auto& b) { return a[0] > b[0]; });
    std::sort(asks.begin(), asks.end(), [](const auto& a, const auto& b) { return a[0] < b[0]; });
    nlohmann::json merged;
    for (const auto& b : bids) merged["bids"].push_back({b[0], b[1]});
    for (const auto& a : asks) merged["asks"].push_back({a[0], a[1]});
    return merged;
}

nlohmann::json OrderBook::buildConsolidatedOrderBook() {
    nlohmann::json consolidated;
    for (const auto& pair : getTopPairs()) {
        std::vector<nlohmann::json> books;
        books.push_back(fetchCoinbaseOrderBook(pair));
        books.push_back(fetchKrakenOrderBook(pair));
        books.push_back(fetchGeminiOrderBook(pair));
        consolidated[pair] = mergeOrderBooks(pair, books);
    }
    return consolidated;
} 