#pragma once

#include <string>
#include <vector>
#include <memory>
#include <spdlog/fmt/fmt.h>
#include "stringutils.hpp"
#include "llmclient.hpp"
#include "tool.hpp"
#include "agentutils.hpp"
#include "greptool.hpp"
#include "readtool.hpp"
#include "writetool.hpp"
#include "treetool.hpp"
#include "gittool.hpp"
#include "rmtool.hpp"

class ConversationLLM {
private:
    std::unique_ptr<LLMClient> client;
    std::vector<std::pair<std::string, std::string>> activeHistory;
    std::string systemPrompt;
    std::vector<std::unique_ptr<agentTool>> tools;
    std::function<void(const std::string&)> respCallback;
    unsigned int maxContext = 0;


public:
    ConversationLLM()
    {

    }

    void loadDefaultTools()
    {
        tools.push_back(std::make_unique<grepTool>());
        tools.push_back(std::make_unique<readFileTool>());
        tools.push_back(std::make_unique<writeToFileTool>());
        tools.push_back(std::make_unique<directoryTreeTool>());
        tools.push_back(std::make_unique<gitTool>());
        tools.push_back(std::make_unique<rmTool>());
    }

    void loadToolsFromFile(const std::string& filename)
    {
        customTool::parseFromFile(filename, tools);
    }

    auto getConversation()
    {
        return activeHistory;
    }

    void setClient(std::unique_ptr<LLMClient> llmclient)
    {
        client = std::move(llmclient);
    }

    void setContextLimit(unsigned int newLimit)
    {
        maxContext = newLimit;
    }

    void setSystemPrompt(const std::string& prompt) {
        systemPrompt = prompt;
    }

    void setPrintCallback(std::function<void(const std::string&)> callback) {
        respCallback = callback;
    }

    template <typename... Args>
    void printResponse(fmt::format_string<Args...> format_str, Args&&... args)
    {
        auto message = fmt::format(format_str, std::forward<Args>(args)...);
        respCallback(message);
    }

    void printResponse(const std::string& message)
    {
        respCallback(message);
    }

    void addUserMessage(const std::string& message) {
        activeHistory.push_back({"user", message});
    }

    void addAssistantMessage(const std::string& message) {
        activeHistory.push_back({"assistant", message});
    }

    unsigned int countContext()
    {
        return stringUtils::countTokensAdvanced(activeHistory);
    }

    // Get full response and automatically add it to history
    std::string getResponse() {
        std::string response = client->chat(activeHistory, systemPrompt);
        auto json = nlohmann::json::parse(response);
        try 
        {
            response = json["message"]["content"];
            addAssistantMessage(response);
        }
        catch(...)
        {
            spdlog::warn("Error parsing LLMClient response");
        }
        return response;
    }

    void clearConversation() {
        activeHistory.clear();
    }

    void removeLastAnswer() {
        if (!activeHistory.empty()) {
            activeHistory.pop_back();
        }
    }

    void compressConversation() {
        spdlog::info("Compressing conversation");
        const static std::string compressPrompt = "Condense our conversation into a single, comprehensive text summary"
        " that captures the core problem, solution approach, and any key technical details or constraints. Focus on the essential"
        " programming challenge and its proposed resolution. You should absolutely skip tool calling parts too.";
        addUserMessage(compressPrompt);
        std::string compressedHistory = getResponse();
        clearConversation();
        addAssistantMessage(compressedHistory);
    }

    void generateAnalysisFile() {
        const std::string prompt = R"(
Create a concise 3-5 sentence summary of our conversation and codebase. Include:
1. What this codebase is about (main purpose/technology)
2. Key components/modules discussed
3. What the system does/produces
4. Your understanding of its core functionality

Keep it brief, clear, and in plain language. Format as a short technical overview for quick reference.
IMPORTANT! Write it into vibecpp.md file! 
)";
        handleUserInput(prompt);
    }

    void processToolCalling(nlohmann::json completionObj)
    {
        spdlog::debug("LLM requested tool calling");
        if(agentUtils::isValidToolCall(completionObj, tools))
        {
            spdlog::debug("Tool calling request is valid");
            printResponse("Tool call: {}", completionObj.dump());
            std::string output = agentUtils::executeToolCall(completionObj, tools);
            printResponse("Tool results: {}", output);
            spdlog::debug("Tool call results: {}", output);
            spdlog::debug("Giving tool calling result back to an LLM");
            handleUserInput(output);
        }
        else
        {
            spdlog::warn("Tool calling request is invalid");
            addUserMessage("Unfortunately, I was not able to process this tool call. Incorrect syntax.");
            handleUserInput("Try again and make sure tool exists and you need it.");
        }
    }

    void handleUserInput(const std::string& message) {
        unsigned int contextLen = countContext();
        spdlog::info("Context: ~{}K tokens", (float)(contextLen / 1000.f));

        if(maxContext > 0 && contextLen > maxContext)
        {
            printResponse("System: reached soft context limit ({}/{}). Compressing conversation", contextLen, maxContext);
            compressConversation();
            contextLen = countContext();
            spdlog::info("Context: ~{}K tokens", (float)(contextLen / 1000.f));
        }

        std::string handlePrompt = R"(You are coding AI assistant that writes code and call tools when needed. You have access to the following tools:
TOOLS:)";

        for (const auto& tool : tools) 
        {
            handlePrompt += tool->getToolInfo() + "\n\n";
        }

        handlePrompt += R"(
INSTRUCTIONS:
1. Analyze the user input carefully
2. If the user input clearly requires using one of the provided tools, respond with EXACTLY ONE JSON object containing:
- "tool_name": exact name from the tool list
- "parameters": object with required arguments
3. If no tool is needed, respond normally without JSON
4. IF TOOL IS NEEDED: RESPOND WITH ONLY JSON - NO TEXT, NO EXPLANATION, NO FORMATTING BEFORE OR AFTER
5. THE JSON MUST START IMMEDIATELY WITH "{"
6. DO NOT INCLUDE ANY WORDS LIKE "I'll", "Let me", "First", "I need" before the JSON
7. DO NOT INCLUDE ANYTHING THAT ISN'T THE TOOL CALL JSON
8. ONLY MAKE ONE TOOL CALL PER RESPONSE - NEVER MULTIPLE TOOL CALLS AT ONCE. If you need to make multiple calls start with just one first.

EXAMPLE TOOL CALLING RESPONSES:
{"tool_name": "grep", "parameters": {"query": "TestClass"}}
{"tool_name": "calculate", "parameters": {"expression": "2+2*3"}}

CRITICAL: NEVER include any text, explanations, or formatting before the JSON response. The response must be pure JSON starting with "{".
ALWAYS RESPOND WITH EXACTLY ONE TOOL CALL PER RESPONSE.
)";

        setSystemPrompt(handlePrompt);
        addUserMessage(message);
        std::string completion = getResponse();
        nlohmann::json completionObj = {};
        if(agentUtils::isToolCalling(completion, completionObj))
        {
            processToolCalling(completionObj);
        }
        else
        {
            spdlog::info("LLM responded:\n\n{}\n\n", completion);
            int toolsAmount = std::max(stringUtils::countSubstring(completion, R"({"tool_name":)"), 
                stringUtils::countSubstring(completion, R"("parameters":)"));
            if(toolsAmount)
            {
                if(toolsAmount > 1)
                {
                    spdlog::warn("System detected multiple tool calls in one response, reverting last answer and prompting it to retry");
                    removeLastAnswer();
                    handleUserInput("Please respond with only ONE tool call at a time!!!");
                }
                else
                {
                    // This is most likely text + 1 tool call
                    if(stringUtils::extractJsonWithBraceMatching(completion, completionObj))
                    {
                        spdlog::warn("System detected text and tool calling within a single response");
                        processToolCalling(completionObj);
                    }
                    else
                    {
                        spdlog::warn("System detected corrupted tool call");
                        handleUserInput("Please make sure you're calling this tool correctly.");
                    }
                }
            }
            else
            {
                printResponse("Agent: {}", completion);
            }
            return;
        }

    }
};