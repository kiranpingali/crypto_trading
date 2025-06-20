#include "coinbase_api.h"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <ctime>

CoinbaseAPI::CoinbaseAPI(const std::string& api_key, const std::string& api_secret, const std::string& passphrase)
    : api_key_(api_key), api_secret_(api_secret), passphrase_(passphrase) {}

CoinbaseAPI::~CoinbaseAPI() {}

std::string CoinbaseAPI::signRequest(const std::string& method, const std::string& request_path, const std::string& body, const std::string& timestamp) const {
    std::string prehash = timestamp + method + request_path + body;
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), api_secret_.c_str(), api_secret_.length(),
                  reinterpret_cast<const unsigned char*>(prehash.c_str()), prehash.length(), NULL, NULL);
    std::ostringstream oss;
    for (int i = 0; i < 32; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return oss.str();
}

size_t CoinbaseAPI::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

nlohmann::json CoinbaseAPI::sendRequest(const std::string& method, const std::string& endpoint, const nlohmann::json& body) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    std::string url = api_url_ + endpoint;
    std::string body_str = body.is_null() ? "" : body.dump();
    std::string timestamp = std::to_string(std::time(nullptr));
    std::string signature = signRequest(method, endpoint, body_str, timestamp);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("CB-ACCESS-KEY: " + api_key_).c_str());
    headers = curl_slist_append(headers, ("CB-ACCESS-SIGN: " + signature).c_str());
    headers = curl_slist_append(headers, ("CB-ACCESS-TIMESTAMP: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("CB-ACCESS-PASSPHRASE: " + passphrase_).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_str.c_str());
    }
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    if (res != CURLE_OK) {
        return nlohmann::json{{"error", curl_easy_strerror(res)}};
    }
    try {
        return nlohmann::json::parse(readBuffer);
    } catch (...) {
        return nlohmann::json{{"error", "Failed to parse JSON"}};
    }
}

nlohmann::json CoinbaseAPI::placeOrder(const std::string& side, const std::string& product_id, double size) {
    nlohmann::json order = {
        {"type", "market"},
        {"side", side},
        {"product_id", product_id},
        {"size", size}
    };
    return sendRequest("POST", "/orders", order);
} 