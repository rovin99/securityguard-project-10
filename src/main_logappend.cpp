#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cstdlib>

class LogEntry {
private:
    long timestamp;    
    std::string token;
    std::string name; 
    std::string role;
    bool arrival;      
    int room;         

public:
    
    LogEntry(long t, std::string tok, std::string n, std::string r, bool a, int rm = -1) 
        : timestamp(t), token(tok), name(n), role(r), arrival(a), room(rm) {}

    
    std::string serialize() const {
        char buffer[100];
        sprintf(buffer, "%ld %s %s %s %s %d\n", 
            timestamp, token.c_str(), role.c_str(), name.c_str(), 
            arrival ? "ARRIVED" : "LEFT", room);
        return std::string(buffer);
    }

    
    long getTimestamp() const { return timestamp; }
    std::string getToken() const { return token; }
};


class LogManager {
private:
    std::string logFile;
    std::string validToken;
    long lastTimestamp;

   
    bool fileExists(const char *filename) {
        std::ifstream infile(filename);
        return infile.good();
    }

public:
   
    LogManager(std::string file) : logFile(file), lastTimestamp(0) {
        if (fileExists(logFile.c_str())) {
            
            std::ifstream infile(logFile.c_str());
            infile >> lastTimestamp >> validToken;
            infile.close();
        } else {
           
            std::ofstream outfile(logFile.c_str(), std::ios::app);
            outfile.close(); 
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
       
        if (entry.getTimestamp() <= lastTimestamp) {
            std::cout << "invalid" << std::endl;
            return false;
        }

        
        if (!validToken.empty() && validToken != entry.getToken()) {
            std::cout << "invalid" << std::endl;
            return false;
        }

        

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
            logFile = argv[i]; 
        }
    }

    // Check if logFile was provided
    if (logFile.empty()) {
        std::cerr << "Log file name is required." << std::endl;
        return 1;
    }

   
    LogEntry entry(timestamp, token, name, role, arrival, room);
    LogManager manager(logFile);
    if (!manager.appendEntry(entry)) {
        exit(255);
    }

    return 0;
}
