#pragma once

#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include "llmclient.hpp"
#include "httpclient.hpp"

class OpenAIClient : public LLMClient, public HttpClient {
private:
    std::string apiKey;
    std::string baseUrl;
public:
    OpenAIClient(const std::string& baseUrl, const std::string& api_key);
    ~OpenAIClient() = default;

    std::string chat(const std::vector<std::pair<std::string, std::string>>& messages,
                     const std::string& systemPrompt = "");

    void setModel(const std::string& model);
};

inline OpenAIClient::OpenAIClient(const std::string& baseURL, const std::string& api_key) : apiKey(api_key), baseUrl(baseURL) {}

inline std::string OpenAIClient::chat(const std::vector<std::pair<std::string, std::string>>& messages,
                                       const std::string& systemPrompt) {
    nlohmann::json payload;
    payload["model"] = getModel();

    nlohmann::json messagesJson;
    if (!systemPrompt.empty()) {
        messagesJson.push_back({{"role", "system"}, {"content", systemPrompt}});
    }

    for (const auto& msg : messages) {
        messagesJson.push_back({{"role", msg.first}, {"content", msg.second}});
    }

    payload["messages"] = messagesJson;

    std::string response = makeRequest(baseUrl, payload, apiKey, true);

    try {
        auto jsonResponse = nlohmann::json::parse(response);
        return jsonResponse["choices"][0].dump();
    } catch (...) {
        return "Error parsing response";
    }
};