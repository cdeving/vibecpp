#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "tool.hpp"

class readFileTool : public agentTool {
public:
    readFileTool() : agentTool(
        "Read File",
        "Reads the contents of a file and returns it as text",
        "file_path: string - path to the file to read",
        "cat "
    ) {}

    std::unique_ptr<agentTool> clone() const override {
        return std::make_unique<readFileTool>(*this);
    }

    std::string executeImpl(const std::string& params) override {
        try {
            nlohmann::json paramJson = nlohmann::json::parse(params);
            std::string filePath = paramJson["file_path"];
            
            std::ifstream file(filePath);
            if (!file.is_open()) {
                return "Error: Could not open file " + filePath;
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            
            return buffer.str();
        } catch (const std::exception& e) {
            return "Error reading file: " + std::string(e.what());
        }
    }
};