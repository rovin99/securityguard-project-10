#pragma once
#include <string>
#include "FileStructure.h"

class FileValidator {
public:
    static bool validateStructure(const std::string& filename);

private:
    static bool checkFileSize(std::ifstream& file);
    static bool validateMagicNumber(std::ifstream& file);
    static bool validateVersion(std::ifstream& file);
    static bool validateContentSize(std::ifstream& file);
};