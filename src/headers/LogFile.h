#pragma once
#include <string>
#include <vector>
#include "LogEntry.h"
#include "Crypto.h"
#include "FileValidator.h"

class LogFile {
public:
    LogFile(const std::string& filename, Crypto& crypto);
    
    bool append(const LogEntry& entry);
    std::vector<LogEntry> readAll();

private:
    std::string filename;
    Crypto& crypto;

    bool writeEncryptedContent(const std::string& content);
    std::string readEncryptedContent();
};