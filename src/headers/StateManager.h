#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include "LogEntry.h"

class StateManager {
public:
    void updateState(const LogEntry& entry);
    std::vector<std::string> getEmployeesInCampus() const;
    std::vector<std::string> getGuestsInCampus() const;
    std::unordered_map<int, std::vector<std::string>> getRoomOccupancy() const;

private:
    std::unordered_map<std::string, bool> employeesInCampus;
    std::unordered_map<std::string, bool> guestsInCampus;
    std::unordered_map<int, std::vector<std::string>> roomOccupancy;
};