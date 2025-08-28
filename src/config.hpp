#pragma once
#include <string>
#include <unordered_map>
#include <any>
#include <fstream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

class ConfigManager {
private:
    nlohmann::json config_data_;
    std::string config_file_path_;

    // Default configuration template
    const nlohmann::json default_config_ = {
        {"client", {
            {"type", "ollama"},
            {"endpoint", "http://127.0.0.1:11434"},
            {"api_key", "empty"},
            {"model", "qwen3"}
        }},
        {"logging", {
            {"level", "none"}
        }}
    };

public:
    explicit ConfigManager(const std::string& config_file_path = ".vibecpp") 
        : config_file_path_(config_file_path) {
    }

    ~ConfigManager() = default;

    bool configFileExists() const {
        return std::filesystem::exists(config_file_path_);
    }

    // Check if config is valid JSON
    bool isConfigValid() const {
        try {
            // Try to parse the current config data
            std::string config_str = config_data_.dump();
            auto parsed = nlohmann::json::parse(config_str);
            return true;
        } catch (const nlohmann::json::exception& e) {
            spdlog::error("Config validation failed: {}", e.what());
            return false;
        }
    }

    bool createConfigFile() {
        config_data_ = default_config_;
        return saveConfig();
    }

    bool loadConfig() {
        try {
            if (!configFileExists()) {
                spdlog::error("Config file '{}' not found", config_file_path_);
                config_data_ = default_config_;
                return false;
            }

            std::ifstream file(config_file_path_);
            if (!file.is_open()) {
                spdlog::error("Failed to open config file: {}", config_file_path_);
                config_data_ = default_config_;
                return false;
            }

            config_data_ = nlohmann::json::parse(file);
            file.close();

            spdlog::info("Configuration loaded successfully from: {}", config_file_path_);
            return true;

        } catch (const nlohmann::json::exception& e) {
            spdlog::error("Failed to parse config file: {}. Error: {}", config_file_path_, e.what());
            spdlog::warn("Using default configuration instead.");
            config_data_ = default_config_;
            return false;
        } catch (const std::exception& e) {
            spdlog::error("Unexpected error loading config: {}", e.what());
            config_data_ = default_config_;
            return false;
        }
    }

    bool saveConfig() const {
        try {
            std::ofstream file(config_file_path_);
            if (!file.is_open()) {
                spdlog::error("Failed to create/open config file for writing: {}", config_file_path_);
                return false;
            }

            file << std::setw(4) << config_data_ << std::endl;
            file.close();
            
            spdlog::info("Configuration saved successfully to: {}", config_file_path_);
            return true;

        } catch (const std::exception& e) {
            spdlog::error("Failed to save config file: {}", e.what());
            return false;
        }
    }

    template<typename T>
    T get(const std::string& key, const T& default_value = T{}) const {
        try {
            // Handle nested keys (e.g., "server.port")
            std::vector<std::string> keys;
            size_t start = 0;
            size_t end = 0;
            
            while ((end = key.find('.', start)) != std::string::npos) {
                keys.push_back(key.substr(start, end - start));
                start = end + 1;
            }
            keys.push_back(key.substr(start));

            // Navigate through nested objects
            const nlohmann::json* current = &config_data_;
            for (const auto& k : keys) {
                current = &current->at(k);
            }

            return current->get<T>();

        } catch (const std::exception& e) {
            spdlog::debug("Config key '{}' not found or invalid, returning default value. Error: {}", 
                         key, e.what());
            return default_value;
        }
    }

    template<typename T>
    void set(const std::string& key, const T& value) {
        try {
            // Handle nested keys
            std::vector<std::string> keys;
            size_t start = 0;
            size_t end = 0;
            
            while ((end = key.find('.', start)) != std::string::npos) {
                keys.push_back(key.substr(start, end - start));
                start = end + 1;
            }
            keys.push_back(key.substr(start));

            // Navigate/create nested objects
            nlohmann::json* current = &config_data_;
            for (size_t i = 0; i < keys.size() - 1; ++i) {
                if (!current->contains(keys[i]) || !current->at(keys[i]).is_object()) {
                    (*current)[keys[i]] = nlohmann::json::object();
                }
                current = &(*current)[keys[i]];
            }

            (*current)[keys.back()] = value;
            spdlog::debug("Config value set: {} = {}", key, nlohmann::json(value).dump());

        } catch (const std::exception& e) {
            spdlog::error("Failed to set config value for key '{}': {}", key, e.what());
        }
    }

    const nlohmann::json& getRawData() const {
        return config_data_;
    }

    void setRawData(const nlohmann::json& data) {
        config_data_ = data;
    }

    bool reload() {
        spdlog::info("Reloading configuration...");
        return loadConfig();
    }

    const std::string& getConfigFilePath() const {
        return config_file_path_;
    }
};
