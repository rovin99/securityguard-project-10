#include "headers/CommandLineParser.h"
#include <stdexcept>
#include <algorithm>

CommandLineArgs CommandLineParser::parse(int argc, char* argv[]) {
    CommandLineArgs args;
    std::vector<std::string> arguments(argv + 1, argv + argc);

    auto it = arguments.begin();
    while (it != arguments.end()) {
        if (*it == "-B") {
            if (++it == arguments.end()) throw std::runtime_error("Missing batch filename");
            args.batchFilename = *it;
            break;  // Ignore all other arguments in batch mode
        } else if (*it == "-T") {
            if (++it == arguments.end()) throw std::runtime_error("Missing token");
            args.token = *it;
        } else if (*it == "-K") {
            if (++it == arguments.end()) throw std::runtime_error("Missing log filename");
            args.logFilename = *it;
        } else if (*it == "-E") {
            if (++it == arguments.end()) throw std::runtime_error("Missing employee name");
            args.employeeName = *it;
        } else if (*it == "-G") {
            if (++it == arguments.end()) throw std::runtime_error("Missing guest name");
            args.guestName = *it;
        } else if (*it == "-A") {
            args.arrival = true;
        } else if (*it == "-L") {
            args.departure = true;
        } else if (*it == "-R") {
            if (++it == arguments.end()) throw std::runtime_error("Missing room ID");
            args.roomId = *it;
        } else {
            throw std::runtime_error("Unknown argument: " + *it);
        }
        ++it;
    }

    validateArgs(args);
    return args;
}

void CommandLineParser::validateArgs(const CommandLineArgs& args) {
    if (args.isBatchMode()) {
        if (args.token.empty() || args.logFilename.empty()) {
            throw std::runtime_error("Batch mode requires -T and -K arguments");
        }
        return;
    }

    if (args.token.empty() || args.logFilename.empty()) {
        throw std::runtime_error("Missing required arguments: -T and -K");
    }

    if (!args.employeeName.has_value() && !args.guestName.has_value()) {
        throw std::runtime_error("Either -E or -G must be specified");
    }

    if (args.employeeName.has_value() && args.guestName.has_value()) {
        throw std::runtime_error("Cannot specify both -E and -G");
    }

    if (!args.arrival && !args.departure) {
        throw std::runtime_error("Either -A or -L must be specified");
    }

    if (args.arrival && args.departure) {
        throw std::runtime_error("Cannot specify both -A and -L");
    }
}