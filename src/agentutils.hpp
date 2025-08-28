#pragma once
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "tool.hpp"
#include "customtool.hpp"

namespace agentUtils
{
    static bool isToolCalling(const std::string &jsonStr, nlohmann::json &json)
    {
        const std::vector<std::string> &requiredProps = {"tool_name", "parameters"};
        try
        {
            json = nlohmann::json::parse(jsonStr);

            // Check if all required properties exist
            for (const auto &prop : requiredProps)
            {
                if (!json.contains(prop))
                {
                    return false;
                }
            }
            return true;
        }
        catch (const nlohmann::json::parse_error &)
        {
            return false; // Invalid JSON
        }
    }

    static bool isValidToolCall(const nlohmann::json &jsonToolCall,
                                const std::vector<std::unique_ptr<agentTool>> &availableTools)
    {
        // Check if JSON has required fields
        if (!jsonToolCall.contains("tool_name") || !jsonToolCall.contains("parameters"))
        {
            return false;
        }

        // Get tool name from JSON
        std::string toolName = jsonToolCall["tool_name"];

        // Check if tool exists in our available tools
        bool toolExists = false;
        for (const auto &tool : availableTools)
        {
            if (tool->getName() == toolName)
            {
                toolExists = true;
                break;
            }
        }

        if (!toolExists)
        {
            return false;
        }

        if (!jsonToolCall["parameters"].is_object())
        {
            return false;
        }

        // If we get here, the tool call is valid
        return true;
    }

    std::string executeToolCall(const nlohmann::json &jsonToolCall,
                                const std::vector<std::unique_ptr<agentTool>> &availableTools)
    {
        // Check if JSON has required fields
        if (!jsonToolCall.contains("tool_name") || !jsonToolCall.contains("parameters"))
        {
            throw std::runtime_error("Invalid tool call: missing tool_name or parameters");
        }

        // Get tool name from JSON
        std::string toolName = jsonToolCall["tool_name"];

        // Find the tool in our available tools
        std::unique_ptr<agentTool> targetTool = nullptr;
        for (const auto &tool : availableTools)
        {
            if (tool->getName() == toolName)
            {
                targetTool = tool->clone();
                break;
            }
        }

        if (!targetTool)
        {
            throw std::runtime_error("Tool not found: " + toolName);
        }

        std::string params = jsonToolCall["parameters"].dump();
        return targetTool->execute(params);
    }

    std::string getHomeDirectory()
    {
        const char *homeDir = std::getenv("HOME");
        if (homeDir == nullptr)
        {
            return "./";
        }
        return std::string(homeDir) + "/";
    }

    void parseFromFile(const std::string &configPath, std::vector<std::unique_ptr<agentTool>>& tools)
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

                if(name.empty() || description.empty() || realCommand.empty())
                {
                    spdlog::error("Failing to parse tool, name, description and realCommand cannot be empty: {}", toolJson.dump());
                    continue;
                }

                spdlog::info("Registered custom tool: {}", name);

                tools.push_back(std::make_unique<customTool>(name, description, arguments, realCommand));
            }
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("Error parsing config: " + std::string(e.what()));
        }
    }
};