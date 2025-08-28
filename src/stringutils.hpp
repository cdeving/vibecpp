#pragma once
#include <string>
#include <regex>
namespace stringUtils
{
    int countSubstring(const std::string &str, const std::string &substr)
    {
        if (substr.empty() || str.empty())
        {
            return 0;
        }

        int count = 0;
        size_t pos = 0;

        while ((pos = str.find(substr, pos)) != std::string::npos)
        {
            count++;
            pos += 1; // Move by 1 to find overlapping matches
        }

        return count;
    }

    bool extractJsonWithBraceMatching(const std::string &input, nlohmann::json &j)
    {
        size_t jsonStart = input.find('{');
        if (jsonStart == std::string::npos)
        {
            // ("No JSON object found");
            return false;
        }

        int braceCount = 0;
        size_t jsonEnd = jsonStart;

        // Properly match braces
        for (size_t i = jsonStart; i < input.length(); ++i)
        {
            if (input[i] == '{')
            {
                braceCount++;
            }
            else if (input[i] == '}')
            {
                braceCount--;
                if (braceCount == 0)
                {
                    jsonEnd = i;
                    break;
                }
            }
        }

        if (braceCount != 0)
        {
            // ("Mismatched braces in JSON");
            return false;
        }

        std::string jsonString = input.substr(jsonStart, jsonEnd - jsonStart + 1);

        try
        {
            j = nlohmann::json::parse(jsonString);
            return true;
        }
        catch (const nlohmann::json::parse_error &e)
        {
            return false;
        }
    }
    std::string truncateString(const std::string &str, size_t maxLength = 20)
    {
        if (maxLength < 3)
        {
            return "..."; // Minimum length needed for "..."
        }

        if (str.length() <= maxLength)
        {
            return str;
        }

        return str.substr(0, maxLength - 3) + "...";
    }
    unsigned int countTokensAdvanced(const std::vector<std::pair<std::string, std::string>> &conv)
    {
        int totalTokens = 0;

        // handles punctuation better
        std::regex tokenRegex(R"(\S+)");

        for (const auto &[role, message] : conv)
        {
            auto tokens_begin = std::sregex_iterator(message.begin(), message.end(), tokenRegex);
            auto tokens_end = std::sregex_iterator();
            int messageTokens = std::distance(tokens_begin, tokens_end);

            totalTokens += messageTokens;
        }

        return totalTokens;
    }
}