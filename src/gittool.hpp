#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "tool.hpp"
#include "cmdexec.hpp"

class gitTool : public agentTool {
public:
    gitTool() : agentTool(
        "git",
        "Executes git commands in the current repository",
        "command: string - git command to execute (clone, pull, push, commit, add ., etc.). ",
        "git"
    ) {}

    std::unique_ptr<agentTool> clone() const override {
        return std::make_unique<gitTool>(*this);
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