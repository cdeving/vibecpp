#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>
#include "config.hpp"
#include "agentutils.hpp"
#include "ollama.hpp"
#include "llmclient.hpp"
#include "oaiclient.hpp"
#include "conversation.hpp"
#include "linenoise.hpp"

bool isPipeInput() {
    struct stat stat_buf;
    if (fstat(STDIN_FILENO, &stat_buf) == 0) {
        return S_ISFIFO(stat_buf.st_mode) || S_ISREG(stat_buf.st_mode);
    }
    return false;
}

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::off);
    spdlog::info("Hello world");

    std::string clientType = {};
    std::string clientEndpoint = {};
    std::string clientApiKey = {};
    std::string clientModel = {};
    std::string outgoingMessage = {};
    bool debugEnabled = false;
    bool disableDefaultTools = false;
    bool disableCustomTools = false;
    bool disableTools = false;

    CLI::App cli{"vibecpp"};
    cli.add_option("--type", clientType, "LLM client type. Supported types: \"ollama\", \"openai\".");
    cli.add_option("--model", clientModel, "LLM name.");
    cli.add_option("--server", clientEndpoint, "LLM API endpoint URL.");
    cli.add_option("--api-key", clientApiKey, "LLM API key.");
    cli.add_flag("--ndt", disableDefaultTools, "Disable default tools.");
    cli.add_flag("--nct", disableCustomTools, "Disable custom tools.");
    cli.add_flag("--nt", disableTools, "Disable all tools.");
    cli.add_flag("-d", debugEnabled, "Enable debug logging mode.");

    try
    {
        cli.parse(argc, argv);
    }
    catch (const CLI::ParseError &e)
    {
        return cli.exit(e);
    }

    if(debugEnabled)
    {
        spdlog::set_level(spdlog::level::debug);
    }

    ConfigManager cfg(agentUtils::getHomeDirectory() + ".vibecpp");
    if(!cfg.loadConfig())
    {
        if(!cfg.createConfigFile())
        {
            std::cout << "Unable to create config. Using default." << std::endl;
        }
    }

    if(clientEndpoint.empty())
        clientEndpoint = cfg.get<std::string>("client.endpoint");
    if(clientApiKey.empty())
        clientApiKey = cfg.get<std::string>("client.api_key");
    if(clientModel.empty())
        clientModel = cfg.get<std::string>("client.model");
    if(clientType.empty())
        clientType = cfg.get<std::string>("client.type");

    if(clientType.empty())
    {
        spdlog::error("Empty client type defined in the config.");
        std::cout << "Empty client type defined in the config." << std::endl << "Supported types: ollama, openai" << std::endl;
        return 1;
    }

    std::unique_ptr<LLMClient> client = {};
    if(clientType == "ollama")
    {
        client = std::make_unique<OllamaClient>(clientEndpoint);
    }
    else if(clientType == "openai")
    {
        client = std::make_unique<OpenAIClient>(clientEndpoint, clientApiKey);
    }
    else
    {
        spdlog::error("Unknown client type defined in the config.");
        std::cout << "Unknown client type defined in the config." << std::endl << "Supported types: ollama, openai" << std::endl;
        return 1;
    }

    client->setModel(clientModel);
    auto conv = std::make_shared<ConversationLLM>();

    if(disableTools)
    {
        disableCustomTools = true;
        disableDefaultTools = true;
    }

    if(!disableDefaultTools)
        conv->loadDefaultTools();

    if(!disableCustomTools)
        conv->loadToolsFromFile(agentUtils::getHomeDirectory() + ".custom.vibecpp");

    conv->setClient(std::move(client));
    conv->setPrintCallback([](const std::string& incoming){
        if(incoming.starts_with("Tool results:"))
            return; // to not spam console with long tool output like read file
        if(incoming.starts_with("Tool call:"))
            std::cout << fmt::format("{:.80}", incoming) << std::endl; // to not spam console with long tool calls like write file
        else
            std::cout << incoming << std::endl;
    });


    if (isPipeInput()) 
    {
        std::getline(std::cin, outgoingMessage, '\0');
        spdlog::info("Starting single message mode, received stdin message: {}", outgoingMessage);
    }
    else 
        spdlog::info("Starting interactive chat mode.");

    if(outgoingMessage.empty()) // chat mode
    {
        linenoise::SetMultiLine(true);
        linenoise::SetHistoryMaxLen(10);
        
        std::cout << fmt::format("Endpoint: {}\nType: {}\nModel: {}", 
            clientEndpoint, clientType, clientModel) << std::endl;
        std::cout << "Ask your first question." << std::endl;
        while (true) 
        {
            std::cout << "==============================" << std::endl;
            std::string input;
            auto quit = linenoise::Readline("You: ", input);
            if (quit) 
            {
                std::cout << "Exiting interactive chat mode..." << std::endl;
                break;
            }
            linenoise::AddHistory(input.c_str());
            conv->handleUserInput(input);
        }
    }
    else //non-chat mode
    {
        conv->handleUserInput(outgoingMessage);
    }
    
    return 0;
}