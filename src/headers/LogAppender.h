#pragma once
#include "LogFile.h"
#include "CommandLineParser.h"

class LogAppender {
public:
    LogAppender(LogFile& logFile);
    bool append(const CommandLineArgs& args);
    bool processBatchFile(const std::string& batchFilename);

private:
    LogFile& logFile;
    bool appendEntry(const CommandLineArgs& args);
};