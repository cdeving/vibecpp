#pragma once
#include <iostream>
#include <string>
#include <memory>
#include "spdlog/spdlog.h"
#include "tool.hpp"
#include "cmdexec.hpp"

class grepTool : public agentTool {
public:
    grepTool() : agentTool(
        "grep",
        R"(Searches through file contents using regular expressions. Works efficiently with codebases of any size. 
        Always specify pattern to search, it is grep with -e flag)",
        "query: string - text or regex to search",
        "grep -rnw ./ -e "
    ) {}

    std::unique_ptr<agentTool> clone() const override {
        return std::make_unique<grepTool>(*this);
    }

    std::string executeImpl(const std::string& params) override {
        auto json = nlohmann::json::parse(params);
        std::string output = CommandExecutor::execute(realCommand, {json["query"].get<std::string>()});
        return output;
    }
};