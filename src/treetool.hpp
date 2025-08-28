#include <string>
#include <nlohmann/json.hpp>
#include "tool.hpp"
#include "cmdexec.hpp"

class directoryTreeTool : public agentTool {
public:
    directoryTreeTool() : agentTool(
        "Directory Tree",
        "Shows current directory structure as a tree",
        "path: string - directory path to show (optional, defaults to current dir)",
        "tree -a  -L 3 -I 'node_modules'"
    ) {}

    std::unique_ptr<agentTool> clone() const override {
        return std::make_unique<directoryTreeTool>(*this);
    }

    std::string executeImpl(const std::string& params) override {
        nlohmann::json paramJson = nlohmann::json::parse(params);
        std::string path = ".";
        
        if (paramJson.contains("path")) {
            path = paramJson["path"];
        }

        std::string output = CommandExecutor::execute(realCommand, {path});
        return output;
    }
};