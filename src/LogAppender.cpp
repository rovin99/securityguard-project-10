#include "LogAppender.h"
#include "LogEntry.h"
#include <fstream>
#include <iostream>

LogAppender::LogAppender(LogFile& logFile) : logFile(logFile) {}

bool LogAppender::append(const CommandLineArgs& args) {
    return appendEntry(args);
}

bool LogAppender::processBatchFile(const std::string& batchFilename) {
    std::ifstream batchFile(batchFilename);
    if (!batchFile.is_open()) {
        std::cerr << "Failed to open batch file: " << batchFilename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(batchFile, line)) {
        CommandLineArgs entryArgs = CommandLineParser::parseFromString(line);
        if (!appendEntry(entryArgs)) {
            return false;
        }
    }

    return true;
}

bool LogAppender::appendEntry(const CommandLineArgs& args) {
    LogEntry entry(args.getPersonType(), args.getEventType(), args.getTimestamp());
    return logFile.append(entry);
}
