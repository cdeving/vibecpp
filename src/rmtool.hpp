#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <iostream>
#include "tool.hpp"
#include "cmdexec.hpp"

class rmTool : public agentTool {
public:
    rmTool() : agentTool(
        "rm",
        "Removes files or directories recursively",
        "command: string - rm command to execute (path, force, recursive, etc) ",
        "rm"
    ) {}

    std::unique_ptr<agentTool> clone() const override {
        return std::make_unique<rmTool>(*this);
    }

    std::string executeImpl(const std::string& params) override {
        try {
            nlohmann::json paramJson = nlohmann::json::parse(params);
            
            std::string command = paramJson.value("command", "");
            
            if (command.empty()) {
                return "Error: No command specified";
            }
            std::string output = CommandExecutor::executeSingleArg(realCommand, {command});
            return output;

        } catch (const std::exception& e) {
            return "Error parsing parameters: " + std::string(e.what());
        }
    }
};