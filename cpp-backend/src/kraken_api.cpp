#include "kraken_api.h"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <cstring>

KrakenAPI::KrakenAPI(const std::string& api_key, const std::string& api_secret)
    : api_key_(api_key), api_secret_(api_secret) {}

KrakenAPI::~KrakenAPI() {}

std::string base64_decode(const std::string& in) {
    BIO* bio, *b64;
    int in_len = in.size();
    int out_len = in_len * 3 / 4;
    std::vector<char> out(out_len);
    bio = BIO_new_mem_buf((void*)in.c_str(), in_len);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int decoded_size = BIO_read(bio, out.data(), in_len);
    BIO_free_all(bio);
    return std::string(out.data(), decoded_size);
}

std::string KrakenAPI::signRequest(const std::string& path, const std::string& nonce, const std::string& postdata) const {
    std::string decoded_secret = base64_decode(api_secret_);
    std::string message = nonce + postdata;
    unsigned char sha256[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)message.c_str(), message.size(), sha256);
    std::string path_sha((char*)sha256, SHA256_DIGEST_LENGTH);
    std::string data = path + path_sha;
    unsigned char* digest;
    digest = HMAC(EVP_sha512(), decoded_secret.c_str(), decoded_secret.length(),
                  (const unsigned char*)data.c_str(), data.length(), NULL, NULL);
    return std::string((char*)digest, SHA512_DIGEST_LENGTH);
}

size_t KrakenAPI::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

nlohmann::json KrakenAPI::sendRequest(const std::string& endpoint, const nlohmann::json& body) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    std::string url = api_url_ + endpoint;
    std::string nonce = std::to_string(std::time(nullptr));
    std::string postdata = "nonce=" + nonce;
    for (auto& el : body.items()) {
        postdata += "&" + el.key() + "=" + el.value().dump();
    }
    std::string signature = signRequest(endpoint, nonce, postdata);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("API-Key: " + api_key_).c_str());
    headers = curl_slist_append(headers, ("API-Sign: " + signature).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str());
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

nlohmann::json KrakenAPI::placeOrder(const std::string& pair, const std::string& type, const std::string& ordertype, double volume) {
    nlohmann::json order = {
        {"pair", pair},
        {"type", type},
        {"ordertype", ordertype},
        {"volume", volume}
    };
    return sendRequest("/0/private/AddOrder", order);
} 