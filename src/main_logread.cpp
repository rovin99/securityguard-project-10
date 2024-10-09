

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
#include <openssl/err.h>
#include <openssl/rand.h>

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

void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

// Derive key from the token using PBKDF2
bool deriveKey(const std::string& token, const std::string& salt, unsigned char* key, size_t keylen) {
    if (PKCS5_PBKDF2_HMAC(token.c_str(), token.size(), 
                           reinterpret_cast<const unsigned char*>(salt.c_str()), salt.size(), 
                           10000, EVP_sha256(), keylen, key) != 1) {
        return false;
    }
    return true;
}

// Decrypt function using AES-GCM
bool decryptLog(const std::string& ciphertext, const unsigned char* key, const unsigned char* iv, 
                const unsigned char* tag, std::string& decryptedText) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    int len;
    int plaintext_len;
    int ret;
    unsigned char plaintext[ciphertext.size()];

    // Initialize decryption context
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
        handleErrors();
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL) != 1)
        handleErrors();
    
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1)
        handleErrors();
    
    if (EVP_DecryptUpdate(ctx, plaintext, &len, reinterpret_cast<const unsigned char*>(ciphertext.c_str()), ciphertext.size()) != 1)
        handleErrors();
    
    plaintext_len = len;

    // Set the expected tag value
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void*)tag) != 1)
        handleErrors();

    // Finalize decryption
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    if (ret > 0) {
        plaintext_len += len;
        decryptedText = std::string(reinterpret_cast<char*>(plaintext), plaintext_len);
    }

    EVP_CIPHER_CTX_free(ctx);
    return ret > 0;
}

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

    bool readLog() {
        std::ifstream file(logFile, std::ios::binary);
        if (!file) return false;

        // Read magic number, version, salt, IV, and authentication tag
        char magicNumber[8], version[4], salt[16], iv[12], tag[16];
        file.read(magicNumber, 8);
        file.read(version, 4);
        file.read(salt, 16);
        file.read(iv, 12);
        file.read(tag, 16);

        // Read the encrypted content
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string ciphertext = buffer.str();

        // Derive the encryption key using PBKDF2
        unsigned char key[32];
        if (!deriveKey(token, std::string(salt, 16), key, sizeof(key))) {
            std::cout << "key derivation failed" << std::endl;
            exit(255);
        }

        // Decrypt the ciphertext
        std::string decryptedText;
        if (!decryptLog(ciphertext, key, (unsigned char*)iv, (unsigned char*)tag, decryptedText)) {
            std::cout << "integrity violation" << std::endl;
            exit(255);
        }

        // Now parse the decrypted log content
        std::istringstream logStream(decryptedText);
        Event event(0, "", "", false, false);
        while (logStream >> event.timestamp >> event.token >> event.name >> event.isEmployee >> event.isArrival >> event.roomId) {
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
        if (!readLog()) {
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

