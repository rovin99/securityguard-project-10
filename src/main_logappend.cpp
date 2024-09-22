#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cstdlib>

class LogEntry {
private:
    long timestamp;    // Time since IITGN opened
    std::string token; // Authentication token
    std::string name;  // Name of employee or guest
    std::string role;  // Employee or guest
    bool arrival;      // Arrival or departure
    int room;          // Room number, -1 if not specified

public:
    // Constructor to create a log entry
    LogEntry(long t, std::string tok, std::string n, std::string r, bool a, int rm = -1) 
        : timestamp(t), token(tok), name(n), role(r), arrival(a), room(rm) {}

    // Serialize the log entry to a string for writing to file
    std::string serialize() const {
        char buffer[100];
        sprintf(buffer, "%ld %s %s %s %s %d\n", 
            timestamp, token.c_str(), role.c_str(), name.c_str(), 
            arrival ? "ARRIVED" : "LEFT", room);
        return std::string(buffer);
    }

    // Getter methods for validation
    long getTimestamp() const { return timestamp; }
    std::string getToken() const { return token; }
};

// LogManager class to handle appending entries to the log file
class LogManager {
private:
    std::string logFile;
    std::string validToken;
    long lastTimestamp;

    // Helper function to check if a file exists
    bool fileExists(const char *filename) {
        std::ifstream infile(filename);
        return infile.good();
    }

public:
    // Constructor to initialize log manager with a log file
    LogManager(std::string file) : logFile(file), lastTimestamp(0) {
        if (fileExists(logFile.c_str())) {
            // If log file exists, read the last timestamp and token
            std::ifstream infile(logFile.c_str());
            infile >> lastTimestamp >> validToken;
            infile.close();
        } else {
            // Create the log file if it does not exist
            std::ofstream outfile(logFile.c_str(), std::ios::app);
            outfile.close(); // Create the file and close immediately
        }
    }

    // Function to append an entry to the log
    bool appendEntry(LogEntry entry) {
        if (!validate(entry)) {
            return false;
        }

        std::ofstream outfile;
        outfile.open(logFile.c_str(), std::ios::app);
        if (!outfile) {
            std::cout << "invalid" << std::endl;
            return false;
        }

        outfile << entry.serialize();
        outfile.close();
        lastTimestamp = entry.getTimestamp(); // Update last timestamp
        return true;
    }

    // Function to validate entry before appending
    bool validate(LogEntry &entry) {
        // Check if the timestamp is valid
        if (entry.getTimestamp() <= lastTimestamp) {
            std::cout << "invalid" << std::endl;
            return false;
        }

        // Check if the token matches (if the log exists)
        if (!validToken.empty() && validToken != entry.getToken()) {
            std::cout << "invalid" << std::endl;
            return false;
        }

        // Additional checks (e.g., room logic, entry consistency) can go here

        return true;
    }
};

// Main function to parse command line arguments and append to log
int main(int argc, char *argv[]) {
    long timestamp = 0;
    std::string token, name, role;
    bool arrival = false;
    int room = -1;
    std::string logFile;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-T") == 0) {
            timestamp = atol(argv[++i]);
        } else if (strcmp(argv[i], "-K") == 0) {
            token = argv[++i];
        } else if (strcmp(argv[i], "-E") == 0) {
            name = argv[++i];
            role = "EMPLOYEE";
        } else if (strcmp(argv[i], "-G") == 0) {
            name = argv[++i];
            role = "GUEST";
        } else if (strcmp(argv[i], "-A") == 0) {
            arrival = true;
        } else if (strcmp(argv[i], "-L") == 0) {
            arrival = false;
        } else {
            logFile = argv[i]; // Assume the last argument is the log file name
        }
    }

    // Check if logFile was provided
    if (logFile.empty()) {
        std::cerr << "Log file name is required." << std::endl;
        return 1;
    }

    // Create a LogEntry and LogManager, then append entry to log
    LogEntry entry(timestamp, token, name, role, arrival, room);
    LogManager manager(logFile);
    if (!manager.appendEntry(entry)) {
        exit(255);
    }

    return 0;
}
