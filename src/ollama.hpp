#pragma once

#include "llmclient.hpp"
#include "httpclient.hpp"
#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

class OllamaClient : public LLMClient, public HttpClient {
private:
    std::string baseUrl;
    nlohmann::json lastResponseInfo;
    float temperature = 0.7;
    int top_k = 20;
    int min_p = 0;
    float top_p = 0.8f;
    float repetition_penalty = 1.05f;

public:
    explicit OllamaClient(const std::string& url = "http://localhost:11434") : baseUrl(url) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~OllamaClient() {
        curl_global_cleanup();
    }

    std::string chat(const std::vector<std::pair<std::string, std::string>>& messages,
                     const std::string& systemPrompt = "") override {
        nlohmann::json payload = {
            {"model", getModel()},
            {"messages", nlohmann::json::array()},
            {"stream", false},
            {"think", false},
            {"options", {
                {"temperature", temperature},
                {"top_k", top_k},
                {"min_p", min_p},
                {"top_p", top_p},
                {"repeat_penalty", repetition_penalty},
            }}
        };

        if (!systemPrompt.empty()) {
            payload["messages"].push_back({
                {"role", "system"},
                {"content", systemPrompt}
            });
        }

        for (const auto& msg : messages) {
            payload["messages"].push_back({
                {"role", msg.first},
                {"content", msg.second}
            });
        }

        return makeRequest(baseUrl + "/api/chat", payload, "", true);
    }

    std::vector<std::string> listModels() override {
        std::string response = makeRequest(baseUrl + "/api/tags", nlohmann::json::object(), "", false);

        if (response.empty()) return {};

        auto json = nlohmann::json::parse(response);
        std::vector<std::string> models;
        for (const auto& model : json["models"]) {
            models.push_back(model["name"]);
        }
        return models;
    }
};