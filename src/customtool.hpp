#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <iostream>
#include "tool.hpp"
#include "cmdexec.hpp"

class customTool : public agentTool {
public:
    customTool(const std::string& name, const std::string& description, 
            const std::string& arguments, const std::string& realCommand)
        : agentTool(name, description, arguments, realCommand) {}

    std::unique_ptr<agentTool> clone() const override {
        return std::make_unique<customTool>(*this);
    }

    std::string executeImpl(const std::string& params) override {
        try {
            nlohmann::json paramJson = nlohmann::json::parse(params);
            std::string command = paramJson.value("command", "");
            std::string output = CommandExecutor::executeSingleArg(realCommand, command);
            return output;

        } catch (const std::exception& e) {
            return "Error parsing parameters: " + std::string(e.what());
        }
    }
};