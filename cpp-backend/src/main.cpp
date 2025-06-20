#include <crow.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <iomanip>
#include <sstream>

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

    // Start server
    app.port(3000).multithreaded().run();

    // Cleanup CURL
    curl_global_cleanup();

    return 0;
} 