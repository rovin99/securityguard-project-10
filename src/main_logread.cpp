#include "headers/LogAppender.h"
#include "headers/CommandLineParser.h"
#include "headers/Crypto.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        CommandLineArgs args = CommandLineParser::parse(argc, argv);
        Crypto crypto(args.token);
        LogFile logFile(args.logFilename, crypto);
        LogAppender appender(logFile);

        bool success;
        if (args.isBatchMode()) {
            success = appender.processBatchFile(args.getBatchFilename());
        } else {
            success = appender.append(args);
        }

        return success ? 0 : 255;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 255;
    }
}