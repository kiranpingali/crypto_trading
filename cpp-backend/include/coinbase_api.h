#pragma once
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class CoinbaseAPI {
public:
    CoinbaseAPI(const std::string& api_key, const std::string& api_secret, const std::string& passphrase);
    ~CoinbaseAPI();

    // Place a market order (buy/sell)
    nlohmann::json placeOrder(const std::string& side, const std::string& product_id, double size);

private:
    std::string api_key_;
    std::string api_secret_;
    std::string passphrase_;
    std::string api_url_ = "https://api.exchange.coinbase.com";

    std::string signRequest(const std::string& method, const std::string& request_path, const std::string& body, const std::string& timestamp) const;
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    nlohmann::json sendRequest(const std::string& method, const std::string& endpoint, const nlohmann::json& body = nullptr);
}; 