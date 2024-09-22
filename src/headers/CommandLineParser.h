#pragma once
#include <string>
#include <vector>
#include <optional>

struct CommandLineArgs {
    std::string token;
    std::string logFilename;
    std::optional<std::string> employeeName;
    std::optional<std::string> guestName;
    std::optional<std::string> roomId;
    bool arrival = false;
    bool departure = false;
    std::string batchFilename;

    bool isBatchMode() const { return !batchFilename.empty(); }
    std::string getBatchFilename() const { return batchFilename; }
};

class CommandLineParser {
public:
    static CommandLineArgs parse(int argc, char* argv[]);

private:
    static void validateArgs(const CommandLineArgs& args);
};