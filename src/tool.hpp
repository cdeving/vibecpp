#pragma once
#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "stringutils.hpp"

class agentTool {
protected:
    std::string name;
    std::string description;
    std::string arguments;
    std::string realCommand;

public:
    agentTool(const std::string& toolName, 
              const std::string& toolDescription, 
              const std::string& toolArguments, 
              const std::string& toolRealCommand)
        : name(toolName), 
          description(toolDescription), 
          arguments(toolArguments), 
          realCommand(toolRealCommand) {}

    virtual ~agentTool() = default;

    std::string execute(const std::string& params) {
        spdlog::info("Calling tool: {} with params: {}", realCommand, stringUtils::truncateString(params, 50));
        return executeImpl(params);
    }

    virtual std::string executeImpl(const std::string& params) = 0;

    virtual std::unique_ptr<agentTool> clone() const = 0;

    std::string getToolInfo() const {
        return "Tool: " + name + "\nDescription: " + description + 
               "\nArguments: " + arguments + "\nCommand: " + realCommand;
    }

    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
    std::string getArguments() const { return arguments; }
    std::string getRealCommand() const { return realCommand; }

};