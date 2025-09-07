#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <sstream>

class CommandExecutor {
public:
    static std::string executeSingleArg(const std::string& command, std::string arg = {}) {
        std::string fullCommand = command + " " + arg;
        return executeRaw(fullCommand);
    }
    static std::string execute(const std::string& command, const std::vector<std::string>& args = {}) {
        // Build the complete command string
        std::ostringstream cmdStream;
        cmdStream << command;
        
        for (const auto& arg : args) {
            // Escape spaces and special characters in arguments
            std::string escapedArg = escapeArgument(arg);
            cmdStream << " " << escapedArg;
        }
        spdlog::info("Executing: {}", cmdStream.str());
        std::string fullCommand = cmdStream.str();
        return executeRaw(fullCommand);
    }
    
    static std::vector<std::string> executeLines(const std::string& command, const std::vector<std::string>& args = {}) {
        std::ostringstream cmdStream;
        cmdStream << command;
        
        for (const auto& arg : args) {
            std::string escapedArg = escapeArgument(arg);
            cmdStream << " " << escapedArg;
        }
        
        std::string fullCommand = cmdStream.str();
        return executeLinesRaw(fullCommand);
    }

private:
    static std::string executeRaw(const std::string& command) {
        spdlog::info("Executing: {}", command);

        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        
        char buffer[128];
        while (fgets(buffer, sizeof buffer, pipe.get()) != nullptr) {
            result += buffer;
        }
        
        return result;
    }
    
    static std::vector<std::string> executeLinesRaw(const std::string& command) {
        std::vector<std::string> lines;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        
        char buffer[128];
        while (fgets(buffer, sizeof buffer, pipe.get()) != nullptr) {
            std::string line(buffer);
            if (!line.empty() && line.back() == '\n') {
                line.pop_back();
            }
            lines.push_back(line);
        }
        
        return lines;
    }
    
    static std::string escapeArgument(const std::string& arg) {
        // Simple escaping for spaces and special characters
        if (arg.find(' ') != std::string::npos || 
            arg.find('"') != std::string::npos ||
            arg.find('\'') != std::string::npos) {
            return "\"" + arg + "\"";
        }
        return arg;
    }
};
