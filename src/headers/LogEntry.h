#pragma once
#include <string>
#include <ctime>

class LogEntry {
public:
    enum class EventType { ARRIVAL, DEPARTURE };
    enum class PersonType { EMPLOYEE, GUEST };

    LogEntry(time_t timestamp, const std::string& name, PersonType personType, 
             EventType eventType, int roomId = -1);
    
    std::string serialize() const;
    static LogEntry deserialize(const std::string& data);

    // Getters
    time_t getTimestamp() const { return timestamp; }
    std::string getName() const { return name; }
    PersonType getPersonType() const { return personType; }
    EventType getEventType() const { return eventType; }
    int getRoomId() const { return roomId; }

private:
    time_t timestamp;
    std::string name;
    PersonType personType;
    EventType eventType;
    int roomId;
};