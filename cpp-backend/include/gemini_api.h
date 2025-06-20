#pragma once
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class GeminiAPI {
public:
    GeminiAPI(const std::string& api_key, const std::string& api_secret);
    ~GeminiAPI();

    // Place a market order (buy/sell)
    nlohmann::json placeOrder(const std::string& symbol, const std::string& side, double amount);

private:
    std::string api_key_;
    std::string api_secret_;
    std::string api_url_ = "https://api.gemini.com";

    std::string signRequest(const std::string& payload) const;
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    nlohmann::json sendRequest(const std::string& endpoint, const nlohmann::json& body);
}; 