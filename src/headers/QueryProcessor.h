#pragma once
#include <string>
#include <vector>
#include "StateManager.h"
#include "LogEntry.h"

class QueryProcessor {
public:
    QueryProcessor(const StateManager& stateManager);
    
    std::string processStateQuery() const;
    std::vector<int> processRoomHistoryQuery(const std::string& name, LogEntry::PersonType type) const;
    int processTotalTimeQuery(const std::string& name, LogEntry::PersonType type) const;

private:
    const StateManager& stateManager;
};