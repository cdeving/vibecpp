#pragma once
#include <string>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include "tool.hpp"
#include "cmdexec.hpp"

class npmTool : public agentTool {
public:
    npmTool() : agentTool(
        "npm",
        "Executes npm commands in the current directory",
        "command: string - npm command to execute (install, run, install, etc.), ",
        "npm"
    ) {}

    std::unique_ptr<agentTool> clone() const override {
        return std::make_unique<npmTool>(*this);
    }

    std::string executeImpl(const std::string& params) override {
        try {
            // Parse parameters from JSON string
            nlohmann::json paramJson = nlohmann::json::parse(params);
            
            std::string command = paramJson.value("command", "");
            
            if (command.empty()) {
                return "Error: No command specified";
            }

            std::string output = CommandExecutor::executeSingleArg(realCommand, {command});
            return output;
        } catch (const std::exception& e) {
            return "Error: " + std::string(e.what());
        }
    }
};