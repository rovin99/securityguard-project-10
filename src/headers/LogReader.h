#pragma once
#include "LogFile.h"
#include "StateManager.h"
#include "QueryProcessor.h"
#include "CommandLineParser.h"

class LogReader {
public:
    LogReader(LogFile& logFile);
    std::string processQuery(const CommandLineArgs& args);

private:
    LogFile& logFile;
    StateManager stateManager;
    QueryProcessor queryProcessor;
};