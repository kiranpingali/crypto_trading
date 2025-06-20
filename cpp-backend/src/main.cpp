#include <crow.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "order_book.h"
#include "coinbase_api.h"
#include "kraken_api.h"
#include "gemini_api.h"

using json = nlohmann::json;

// Structure to hold stock data
struct StockData {
    std::string date;
    double open;
    double high;
    double low;
    double close;
};

// Structure to hold order data
struct Order {
    std::string id;
    std::string symbol;
    int quantity;
    std::string type;
    double price;
    double total;
    std::string timestamp;
    std::string status;
};

// Global variables
std::vector<Order> orderBook;
std::map<std::string, double> lastPrices;

// Callback function for CURL to write response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to get current price from Yahoo Finance
double getCurrentPrice(const std::string& symbol) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + symbol;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            try {
                json data = json::parse(response);
                double price = data["chart"]["result"][0]["meta"]["regularMarketPrice"];
                return price;
            } catch (const std::exception& e) {
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                return 0.0;
            }
        }
    }
    return 0.0;
}

// Function to get historical data from Yahoo Finance
std::vector<StockData> getHistoricalData(const std::string& symbol) {
    CURL* curl = curl_easy_init();
    std::string response;
    std::vector<StockData> data;

    if (curl) {
        // Calculate time range (last 250 days)
        auto now = std::chrono::system_clock::now();
        auto end = std::chrono::system_clock::to_time_t(now);
        auto start = end - (250 * 24 * 60 * 60); // 250 days ago

        std::stringstream ss;
        ss << "https://query1.finance.yahoo.com/v8/finance/chart/" << symbol
           << "?period1=" << start << "&period2=" << end << "&interval=1d";

        curl_easy_setopt(curl, CURLOPT_URL, ss.str().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            try {
                json j = json::parse(response);
                auto timestamps = j["chart"]["result"][0]["timestamp"];
                auto quotes = j["chart"]["result"][0]["indicators"]["quote"][0];

                for (size_t i = 0; i < timestamps.size(); ++i) {
                    StockData stock;
                    stock.date = std::to_string(timestamps[i]);
                    stock.open = quotes["open"][i];
                    stock.high = quotes["high"][i];
                    stock.low = quotes["low"][i];
                    stock.close = quotes["close"][i];
                    data.push_back(stock);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            }
        }
    }
    return data;
}

int main() {
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Create Crow app
    crow::SimpleApp app;

    // CORS middleware
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
        .global()
        .headers("Content-Type")
        .methods("GET"_method, "POST"_method);

    // API endpoint for stock data
    CROW_ROUTE(app, "/api/stock/<string>")
        .methods("GET"_method)
        ([](const std::string& symbol) {
            try {
                auto data = getHistoricalData(symbol);
                json response;
                for (const auto& stock : data) {
                    json item;
                    item["date"] = stock.date;
                    item["open"] = stock.open;
                    item["high"] = stock.high;
                    item["low"] = stock.low;
                    item["close"] = stock.close;
                    response.push_back(item);
                }
                return crow::response(response.dump());
            } catch (const std::exception& e) {
                return crow::response(500, json{{"error", e.what()}}.dump());
            }
        });

    // API endpoint for order submission
    CROW_ROUTE(app, "/api/order")
        .methods("POST"_method)
        ([](const crow::request& req) {
            try {
                auto x = json::parse(req.body);
                Order order;
                order.id = std::to_string(std::time(nullptr));
                order.symbol = x["symbol"];
                order.quantity = x["quantity"];
                order.type = x["type"];
                order.timestamp = x["timestamp"];
                
                // Get current price
                order.price = getCurrentPrice(order.symbol);
                order.total = order.price * order.quantity;
                order.status = "EXECUTED";

                // Store order
                orderBook.push_back(order);
                lastPrices[order.symbol] = order.price;

                json response;
                response["orderId"] = order.id;
                response["symbol"] = order.symbol;
                response["quantity"] = order.quantity;
                response["type"] = order.type;
                response["price"] = order.price;
                response["total"] = order.total;
                response["timestamp"] = order.timestamp;
                response["status"] = order.status;

                return crow::response(response.dump());
            } catch (const std::exception& e) {
                return crow::response(500, json{{"error", e.what()}}.dump());
            }
        });

    // API endpoint for order history
    CROW_ROUTE(app, "/api/orders")
        .methods("GET"_method)
        ([]() {
            json response;
            for (const auto& order : orderBook) {
                json item;
                item["id"] = order.id;
                item["symbol"] = order.symbol;
                item["quantity"] = order.quantity;
                item["type"] = order.type;
                item["price"] = order.price;
                item["total"] = order.total;
                item["timestamp"] = order.timestamp;
                item["status"] = order.status;
                response.push_back(item);
            }
            return crow::response(response.dump());
        });

    // API endpoint for last price
    CROW_ROUTE(app, "/api/price/<string>")
        .methods("GET"_method)
        ([](const std::string& symbol) {
            auto it = lastPrices.find(symbol);
            if (it == lastPrices.end()) {
                return crow::response(404, json{{"error", "No price data available"}}.dump());
            }
            json response;
            response["symbol"] = symbol;
            response["price"] = it->second;
            return crow::response(response.dump());
        });

    // API endpoint for trading on best price
    CROW_ROUTE(app, "/api/trade").methods("POST"_method)
    ([&](const crow::request& req) {
        try {
            auto x = json::parse(req.body);
            std::string pair = x["pair"];
            std::string side = x["side"];
            double quantity = x["quantity"];

            // Initialize exchange APIs (replace with real keys/secrets in production)
            CoinbaseAPI coinbase("API_KEY", "API_SECRET", "PASSPHRASE");
            KrakenAPI kraken("API_KEY", "API_SECRET");
            GeminiAPI gemini("API_KEY", "API_SECRET");
            OrderBook ob(&coinbase, &kraken, &gemini);
            auto consolidated = ob.buildConsolidatedOrderBook();
            auto book = consolidated[pair];

            // Find best price and exchange
            double best_price = 0;
            std::string best_exchange;
            if (side == "buy") {
                best_price = 1e12;
                // Check asks
                if (!book["asks"].empty()) {
                    for (const auto& ask : book["asks"]) {
                        if (ask[0] < best_price) best_price = ask[0];
                    }
                }
            } else {
                best_price = 0;
                // Check bids
                if (!book["bids"].empty()) {
                    for (const auto& bid : book["bids"]) {
                        if (bid[0] > best_price) best_price = bid[0];
                    }
                }
            }

            // Determine which exchange has the best price
            std::string exchanges[3] = {"coinbase", "kraken", "gemini"};
            double prices[3] = {0, 0, 0};
            // Fetch top-of-book from each exchange
            prices[0] = (side == "buy") ? ob.fetchCoinbaseOrderBook(pair)["asks"][0][0] : ob.fetchCoinbaseOrderBook(pair)["bids"][0][0];
            prices[1] = (side == "buy") ? ob.fetchKrakenOrderBook(pair)["asks"][0][0] : ob.fetchKrakenOrderBook(pair)["bids"][0][0];
            prices[2] = (side == "buy") ? ob.fetchGeminiOrderBook(pair)["asks"][0][0] : ob.fetchGeminiOrderBook(pair)["bids"][0][0];
            int best_idx = 0;
            for (int i = 1; i < 3; ++i) {
                if ((side == "buy" && prices[i] < prices[best_idx]) || (side == "sell" && prices[i] > prices[best_idx])) {
                    best_idx = i;
                }
            }
            best_exchange = exchanges[best_idx];

            // Execute order on the best exchange
            json exec_result;
            if (best_exchange == "coinbase") {
                exec_result = coinbase.placeOrder(side, pair, quantity);
            } else if (best_exchange == "kraken") {
                exec_result = kraken.placeOrder(pair, side, "market", quantity);
            } else if (best_exchange == "gemini") {
                exec_result = gemini.placeOrder(pair, side, quantity);
            }

            json response;
            response["pair"] = pair;
            response["side"] = side;
            response["quantity"] = quantity;
            response["best_price"] = prices[best_idx];
            response["exchange"] = best_exchange;
            response["execution"] = exec_result;
            return crow::response(response.dump());
        } catch (const std::exception& e) {
            return crow::response(500, json{{"error", e.what()}}.dump());
        }
    });

    // Start server
    app.port(3000).multithreaded().run();

    // Cleanup CURL
    curl_global_cleanup();

    return 0;
} 