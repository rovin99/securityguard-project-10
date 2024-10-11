#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>


struct Event {
    long timestamp;
    std::string token;
    std::string name;
    bool isEmployee;
    bool isArrival;
    int roomId;

    Event(long t, const std::string& tok, const std::string& n, bool emp, bool arr, int room = -1)
        : timestamp(t), token(tok), name(n), isEmployee(emp), isArrival(arr), roomId(room) {}
};

class LogReader {
private:
    std::string logFile;
    std::string token;
    std::vector<Event> events;
    std::map<std::string, bool> inCampus;
    std::map<std::string, int> currentRoom;
    std::map<std::string, std::vector<int>> roomHistory;
    std::map<std::string, long> totalTime;
    std::map<std::string, long> lastEntry;
    // Instead of deriving the key, use a static key for testing
    unsigned char key[32] = { 0x93, 0xb, 0x5f, 0x24, 0x61, 0x7a, 0xf9, 0x48, 0x60, 0x96, 0x88, 0x12, 0xe, 0x57, 0x92, 0x73,
                              0xe, 0xf5, 0x10, 0xa2, 0xe3, 0x23, 0x23, 0x7f, 0x2f, 0xdf, 0x18, 0x24, 0xab, 0x49, 0x73, 0xf8 };


        unsigned char salt[16];
        unsigned char iv[12];   // IV for AES-GCM

        
void deriveKey() {
    

    PKCS5_PBKDF2_HMAC(token.c_str(), token.length(), salt, 16, 100, EVP_sha256(), 32, key);

  
}
        bool readAndDecryptLog() {
            std::ifstream file(logFile, std::ios::binary);
            if (!file) {
                std::cerr << "Error: Unable to open file" << std::endl;
                return false;
            }

            char magic[8];
            uint32_t version;
            file.read(magic, 8);
            file.read(reinterpret_cast<char*>(&version), sizeof(version));
            file.read(reinterpret_cast<char*>(salt), sizeof(salt));
            file.read(reinterpret_cast<char*>(iv), sizeof(iv));

            if (!file) {
                std::cerr << "Error: File header read failed" << std::endl;
                return false;
            }

            // Verify magic number
            if (std::string(magic, 8) != "SECURLOG") {
                std::cerr << "Error: Invalid magic number" << std::endl;
                return false;
            }

            std::cout << "Magic number: " << std::string(magic, 8) << std::endl;
            std::cout << "Version: " << version << std::endl;
            std::cout << "Salt: ";
            for (int i = 0; i < 16; i++) std::cout << std::hex << (int)salt[i] << " ";
            std::cout << std::endl;
            std::cout << "IV: ";
            for (int i = 0; i < 12; i++) std::cout << std::hex << (int)iv[i] << " ";
            std::cout << std::endl;

            // Read the rest of the file
            std::vector<unsigned char> encrypted_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            std::cout << "Encrypted content size: " << encrypted_content.size() << std::endl;



            if (encrypted_content.size() < 16) {
                std::cerr << "Error: Encrypted content too short" << std::endl;
                return false;
            }

            // The last 16 bytes are the tag
            unsigned char tag[16];
            std::copy(encrypted_content.end() - 16, encrypted_content.end(), tag);
            encrypted_content.resize(encrypted_content.size() - 16);

            std::cout << "Tag: ";
            for (int i = 0; i < 16; i++) std::cout << std::hex << (int)tag[i] << " ";
            std::cout << std::endl;

           
            std::cout << "Derived key: ";
            for (int i = 0; i < 32; i++) std::cout << std::hex << (int)key[i] << " ";
            std::cout << std::endl;

            // Set up the decryption context
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                std::cerr << "Error: Unable to create cipher context" << std::endl;
                return false;
            }

            if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
                std::cerr << "Error: Decryption initialization failed" << std::endl;
                EVP_CIPHER_CTX_free(ctx);
                return false;
            }

            // Decrypt the content
            std::vector<unsigned char> decrypted_content(encrypted_content.size());
            int len;
            if (EVP_DecryptUpdate(ctx, decrypted_content.data(), &len, encrypted_content.data(), encrypted_content.size()) != 1) {
                std::cerr << "Error: Decryption update failed" << std::endl;
                EVP_CIPHER_CTX_free(ctx);
                return false;
            }
            int plaintext_len = len;

            // Set the expected tag value
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag) != 1) {
                std::cerr << "Error: Setting GCM tag failed" << std::endl;
                EVP_CIPHER_CTX_free(ctx);
                return false;
            }

            // Finalize the decryption
            int ret = EVP_DecryptFinal_ex(ctx, decrypted_content.data() + len, &len);
            if (ret <= 0) {
                std::cerr << "Error: Decryption failed or integrity check failed" << std::endl;
                EVP_CIPHER_CTX_free(ctx);
                return false;
            }

            plaintext_len += len;
            decrypted_content.resize(plaintext_len);

            EVP_CIPHER_CTX_free(ctx);

            std::cout << "Decrypted content: " << std::string(decrypted_content.begin(), decrypted_content.end()) << std::endl;

            // Parse the decrypted content
            std::string decrypted_str(decrypted_content.begin(), decrypted_content.end());
            std::istringstream iss(decrypted_str);
            std::string line;
            while (std::getline(iss, line)) {
                Event event(0, "", "", false, false);
                std::istringstream line_iss(line);
                line_iss >> event.timestamp >> event.token >> event.name >> event.isEmployee >> event.isArrival >> event.roomId;
                if (event.token != token) {
                    std::cerr << "Error: Token mismatch in log entry" << std::endl;
                    return false;
                }
                events.push_back(event);
                updateState(event);
            }
            return true;
        }
    bool readLog() {
        std::ifstream file(logFile);
        if (!file) return false;

        Event event(0, "", "", false, false);
        while (file >> event.timestamp >> event.token >> event.name >> event.isEmployee >> event.isArrival >> event.roomId) {
            if (event.token != token) {
                std::cout << "integrity violation" << std::endl;
                exit(255);
            }
            events.push_back(event);
            updateState(event);
        }
        return true;
    }

    void updateState(const Event& event) {
        std::string key = (event.isEmployee ? "E:" : "G:") + event.name;
        if (event.isArrival) {
            if (event.roomId == -1) {
                inCampus[key] = true;
                lastEntry[key] = event.timestamp;
            } else {
                currentRoom[key] = event.roomId;
                roomHistory[key].push_back(event.roomId);
            }
        } else {
            if (event.roomId == -1) {
                inCampus[key] = false;
                totalTime[key] += event.timestamp - lastEntry[key];
                currentRoom.erase(key);
            } else {
                currentRoom.erase(key);
            }
        }
    }

    void printCurrentState() {
        std::set<std::string> employees, guests;
        std::map<int, std::set<std::string>> rooms;

        for (const auto& pair : inCampus) {
            if (pair.second) {
                if (pair.first[0] == 'E') {
                    employees.insert(pair.first.substr(2));
                } else {
                    guests.insert(pair.first.substr(2));
                }
            }
        }

        for (const auto& pair : currentRoom) {
            rooms[pair.second].insert(pair.first.substr(2));
        }

        std::cout << join(employees, ",") << std::endl;
        std::cout << join(guests, ",") << std::endl;

        for (const auto& room : rooms) {
            std::cout << room.first << ": " << join(room.second, ",") << std::endl;
        }
    }

    void printRoomHistory(const std::string& name, bool isEmployee) {
        std::string key = (isEmployee ? "E:" : "G:") + name;
        if (roomHistory.count(key) > 0) {
            std::cout << join(roomHistory[key], ",") << std::endl;
        }
    }

    void printTotalTime(const std::string& name, bool isEmployee) {
        std::string key = (isEmployee ? "E:" : "G:") + name;
        long time = totalTime[key];
        if (inCampus[key]) {
            time += events.back().timestamp - lastEntry[key];
        }
        if (time > 0) {
            std::cout << time << std::endl;
        }
    }

    void printIntersection(const std::vector<std::string>& names) {
        std::map<int, std::set<long>> roomOccupancy;
        for (const auto& event : events) {
            std::string key = (event.isEmployee ? "E:" : "G:") + event.name;
            if (event.roomId != -1) {
                if (event.isArrival) {
                    roomOccupancy[event.roomId].insert(event.timestamp);
                } else {
                    roomOccupancy[event.roomId].erase(event.timestamp);
                }
            }
        }

        std::set<int> commonRooms;
        for (const auto& room : roomOccupancy) {
            bool allPresent = true;
            for (const auto& name : names) {
                if (roomHistory[name].end() == std::find(roomHistory[name].begin(), roomHistory[name].end(), room.first)) {
                    allPresent = false;
                    break;
                }
            }
            if (allPresent) {
                commonRooms.insert(room.first);
            }
        }

        if (!commonRooms.empty()) {
            std::cout << join(std::vector<int>(commonRooms.begin(), commonRooms.end()), ",") << std::endl;
        }
    }

    template<typename T>
    std::string join(const T& elements, const std::string& delimiter) {
        std::ostringstream os;
        auto it = elements.begin();
        if (it != elements.end()) {
            os << *it++;
        }
        while (it != elements.end()) {
            os << delimiter << *it++;
        }
        return os.str();
    }

public:
    LogReader(const std::string& file, const std::string& tok) : logFile(file), token(tok) {
            deriveKey();
            if (!readAndDecryptLog()) {
                std::cout << "invalid" << std::endl;
                exit(255);
            }
        }
    void processCommand(int argc, char* argv[]) {
        bool stateQuery = false, roomQuery = false, timeQuery = false, intersectionQuery = false;
        std::string queryName;
        bool queryIsEmployee = false;
        std::vector<std::string> intersectionNames;

        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "-S") == 0) stateQuery = true;
            else if (strcmp(argv[i], "-R") == 0) roomQuery = true;
            else if (strcmp(argv[i], "-T") == 0) timeQuery = true;
            else if (strcmp(argv[i], "-I") == 0) intersectionQuery = true;
            else if (strcmp(argv[i], "-E") == 0) {
                if (roomQuery || timeQuery) {
                    queryName = argv[++i];
                    queryIsEmployee = true;
                } else if (intersectionQuery) {
                    intersectionNames.push_back("E:" + std::string(argv[++i]));
                }
            }
            else if (strcmp(argv[i], "-G") == 0) {
                if (roomQuery || timeQuery) {
                    queryName = argv[++i];
                    queryIsEmployee = false;
                } else if (intersectionQuery) {
                    intersectionNames.push_back("G:" + std::string(argv[++i]));
                }
            }
        }

        if (stateQuery) printCurrentState();
        else if (roomQuery) printRoomHistory(queryName, queryIsEmployee);
        else if (timeQuery) printTotalTime(queryName, queryIsEmployee);
        else if (intersectionQuery) {
            if (intersectionNames.empty()) {
                std::cout << "unimplemented" << std::endl;
            } else {
                printIntersection(intersectionNames);
            }
        }
        else {
            std::cout << "invalid" << std::endl;
            exit(255);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "invalid" << std::endl;
        return 255;
    }

    std::string token, logFile;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-K") == 0) token = argv[++i];
        else logFile = argv[i];
    }

    if (token.empty() || logFile.empty()) {
        std::cout << "invalid" << std::endl;
        return 255;
    }

    LogReader reader(logFile, token);
    reader.processCommand(argc, argv);

    return 0;
}