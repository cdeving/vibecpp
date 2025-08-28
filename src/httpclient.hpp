#pragma once

#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <spdlog/spdlog.h>


class HttpClient {
public:
    virtual ~HttpClient() = default;

protected:
    std::string makeRequest(const std::string& url, const nlohmann::json& payload, const std::string& api_key = "", bool isPost = true) {
        std::string response;
        CURL* curl = curl_easy_init();
        if (!curl) return {};

        std::string jsonStr = payload.dump();
        spdlog::debug("makeRequest payload: {}", jsonStr);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        if (isPost) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        struct curl_slist* headers = createHeaders(api_key);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            return "";
        }
        spdlog::debug("makeRequest response: {}", response);
        return response;
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size * nmemb;
        userp->append((char*)contents, totalSize);
        return totalSize;
    }

    struct curl_slist* createHeaders(const std::string& api_key = "") {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!api_key.empty()) {
            headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
        }
        return headers;
    }
};