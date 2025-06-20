#pragma once
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class KrakenAPI {
public:
    KrakenAPI(const std::string& api_key, const std::string& api_secret);
    ~KrakenAPI();

    // Place a market order (buy/sell)
    nlohmann::json placeOrder(const std::string& pair, const std::string& type, const std::string& ordertype, double volume);

private:
    std::string api_key_;
    std::string api_secret_;
    std::string api_url_ = "https://api.kraken.com";

    std::string signRequest(const std::string& path, const std::string& nonce, const std::string& postdata) const;
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    nlohmann::json sendRequest(const std::string& endpoint, const nlohmann::json& body);
}; 