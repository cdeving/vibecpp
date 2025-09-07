#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <filesystem>
#include <iostream>
#include "tool.hpp"
#include "cmdexec.hpp"
#include <spdlog/fmt/bundled/args.h>

class JSONObjectFormatter
{
public:
    static std::string jsonValueToString(const nlohmann::json &value)
    {
        switch (value.type())
        {
        case nlohmann::json::value_t::string:
            return value.get<std::string>();
        case nlohmann::json::value_t::number_integer:
            return std::to_string(value.get<int64_t>());
        case nlohmann::json::value_t::number_unsigned:
            return std::to_string(value.get<uint64_t>());
        case nlohmann::json::value_t::number_float:
            return fmt::format("{:.6g}", value.get<double>());
        case nlohmann::json::value_t::boolean:
            return value.get<bool>() ? "true" : "false";
        case nlohmann::json::value_t::null:
            return "null";
        case nlohmann::json::value_t::array:
        case nlohmann::json::value_t::object:
            return value.dump();
        default:
            return "null";
        }
    }
    static std::vector<std::string> extractKeys(const std::string &format_str)
    {
        std::vector<std::string> keys;
        std::unordered_set<std::string> unique_keys;
        size_t pos = 0;

        while ((pos = format_str.find('{', pos)) != std::string::npos)
        {
            size_t end_pos = format_str.find('}', pos);
            if (end_pos != std::string::npos)
            {
                std::string key = format_str.substr(pos + 1, end_pos - pos - 1);

                // Remove format specifiers (everything after ':')
                size_t colon_pos = key.find(':');
                if (colon_pos != std::string::npos)
                {
                    key = key.substr(0, colon_pos);
                }

                if (!key.empty() && unique_keys.find(key) == unique_keys.end())
                {
                    unique_keys.insert(key);
                    keys.push_back(key);
                }
                pos = end_pos + 1;
            }
            else
            {
                break;
            }
        }

        return keys;
    }
    static std::string format(const std::string &format_str, const nlohmann::json &data)
    {
        auto requiredKeys = extractKeys(format_str);
        fmt::dynamic_format_arg_store<fmt::format_context> store;
        for(const auto& key : requiredKeys)
        {
            // if not found - replace with empty placeholder
            store.push_back(fmt::arg(key.c_str(), data.contains(key) ? jsonValueToString(data[key]) : ""));
        }
        return fmt::vformat(format_str, store);
    }
};

class customTool : public agentTool
{
private:
    std::string callFormat = {};
    bool multiArg = false;

public:
    customTool(const std::string &name, const std::string &description,
               const std::string &arguments, const std::string &realCommand, const std::string &format)
        : agentTool(name, description, arguments, realCommand), callFormat(format) {}

    std::unique_ptr<agentTool> clone() const override
    {
        return std::make_unique<customTool>(*this);
    }

    std::string executeFormatted(const std::string &command)
    {
        nlohmann::json commandJson = nlohmann::json::parse(command);
        std::string finalCommand = realCommand + " " + JSONObjectFormatter::format(callFormat, commandJson);
        std::string output = CommandExecutor::executeSingleArg(finalCommand);
        return output;
    }

    std::string executeImpl(const std::string &params) override
    {
        try
        {
            nlohmann::json paramJson = nlohmann::json::parse(params);
            std::string command = paramJson.contains("command") ? paramJson["command"].dump() : "";
            return executeFormatted(command);
        }
        catch (const std::exception &e)
        {
            return "Error parsing parameters: " + std::string(e.what());
        }
    }

    static void parseFromFile(const std::string &configPath, std::vector<std::unique_ptr<agentTool>> &tools)
    {
        try
        {
            std::ifstream file(configPath);
            if (!file.is_open())
            {
                spdlog::error("Could not open tools config file.");
                return;
            }

            nlohmann::json config = nlohmann::json::parse(file);

            for (const auto &toolJson : config["tools"])
            {
                std::string name = toolJson.value("name", "");
                std::string description = toolJson.value("description", "");
                std::string arguments = toolJson.value("arguments", "");
                std::string realCommand = toolJson.value("realCommand", "");
                std::string formatting = toolJson.value("format", "");

                if (name.empty() || description.empty() || realCommand.empty() || formatting.empty())
                {
                    spdlog::error("Failing to parse tool, name, description, realCommand and formatting cannot be empty: {}", toolJson.dump());
                    continue;
                }

                arguments = "JSON OBJECT named \"command\" with following members: " + arguments;

                spdlog::info("Registered custom tool: {}", name);

                tools.push_back(std::make_unique<customTool>(name, description, arguments, realCommand, formatting));
            }
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("Error parsing config: " + std::string(e.what()));
        }
    }
};