#pragma once

#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

class LLMClient {
private:
    std::string modelName;
public:
    virtual ~LLMClient() = default;

    virtual std::string chat(const std::vector<std::pair<std::string, std::string>>& messages,
                             const std::string& systemPrompt = "") = 0;

    void setModel(const std::string& model) {
        modelName = model;
    }

    std::string getModel() {
        return modelName;
    }

    virtual std::vector<std::string> listModels() {
        // Default implementation - return empty vector
        return {};
    }
};