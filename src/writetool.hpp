#include <string>
#include <fstream>
#include <iostream>
#include "tool.hpp"

class writeToFileTool : public agentTool {
public:
    writeToFileTool() : agentTool(
        "Write to File",
        "Writes content to a specified file",
        "file_path: string - path to the file, content: string - content to write, append: boolean - whether to append to file",
        "fwrite(file_path, content, append)"
    ) {}

    std::unique_ptr<agentTool> clone() const override {
        return std::make_unique<writeToFileTool>(*this);
    }

    std::string executeImpl(const std::string& params) override {
        try {
            auto jsonParams = nlohmann::json::parse(params);
            
            std::string filePath = jsonParams["file_path"];
            std::string content = jsonParams["content"];
            bool append = jsonParams.value("append", false);
            
            // Open file with appropriate mode
            std::ofstream file(filePath, append ? std::ios::app : std::ios::out);
            
            if (!file.is_open()) {
                return "Error: Could not open file " + filePath;
            }
            
            file << content;
            file.close();
            
            return "Successfully wrote to file: " + filePath;
        } catch (const std::exception& e) {
            return "Error: " + std::string(e.what());
        }
    }
};