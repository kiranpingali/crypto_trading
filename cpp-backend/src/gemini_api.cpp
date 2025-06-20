#include "gemini_api.h"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <cstring>
#include <base64.h> // If not available, implement base64 encoding inline

GeminiAPI::GeminiAPI(const std::string& api_key, const std::string& api_secret)
    : api_key_(api_key), api_secret_(api_secret) {}

GeminiAPI::~GeminiAPI() {}

std::string base64_encode(const unsigned char* input, int length) {
    BIO *bmem, *b64;
    BUF_MEM *bptr;
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    std::string encoded(bptr->data, bptr->length);
    BIO_free_all(b64);
    return encoded;
}

std::string GeminiAPI::signRequest(const std::string& payload) const {
    unsigned char* digest;
    digest = HMAC(EVP_sha384(), api_secret_.c_str(), api_secret_.length(),
                  reinterpret_cast<const unsigned char*>(payload.c_str()), payload.length(), NULL, NULL);
    return base64_encode(digest, 48); // SHA384 digest is 48 bytes
}

size_t GeminiAPI::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

nlohmann::json GeminiAPI::sendRequest(const std::string& endpoint, const nlohmann::json& body) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    std::string url = api_url_ + endpoint;
    std::string payload = body.dump();
    std::string b64_payload = base64_encode(reinterpret_cast<const unsigned char*>(payload.c_str()), payload.length());
    std::string signature = signRequest(b64_payload);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("X-GEMINI-APIKEY: " + api_key_).c_str());
    headers = curl_slist_append(headers, ("X-GEMINI-PAYLOAD: " + b64_payload).c_str());
    headers = curl_slist_append(headers, ("X-GEMINI-SIGNATURE: " + signature).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
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

nlohmann::json GeminiAPI::placeOrder(const std::string& symbol, const std::string& side, double amount) {
    std::string endpoint = "/v1/order/new";
    nlohmann::json order = {
        {"request", endpoint},
        {"nonce", std::to_string(std::time(nullptr) * 1000)},
        {"symbol", symbol},
        {"amount", std::to_string(amount)},
        {"side", side},
        {"type", "exchange market"}
    };
    return sendRequest(endpoint, order);
} 