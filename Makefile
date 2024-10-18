# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src/headers

# Linker flags for OpenSSL
LDFLAGS = $(shell pkg-config --libs openssl)

# Source files
SRCS = src/main_logappend.cpp src/main_logread.cpp
OBJS = $(SRCS:.cpp=.o)

# Output binaries
TARGET_LOGAPPEND = logappend
TARGET_LOGREAD = logread

# Default target
all: $(TARGET_LOGAPPEND) $(TARGET_LOGREAD)

# Rule for logappend
$(TARGET_LOGAPPEND): src/main_logappend.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Rule for logread
$(TARGET_LOGREAD): src/main_logread.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f src/*.o $(TARGET_LOGAPPEND) $(TARGET_LOGREAD)

# Installation step (optional)
install: all
	install -m 755 $(TARGET_LOGAPPEND) /usr/local/bin/
	install -m 755 $(TARGET_LOGREAD) /usr/local/bin/

.PHONY: all clean install
