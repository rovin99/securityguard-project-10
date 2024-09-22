#pragma once
#include <cstdint>
#include <array>

struct FileHeader {
    static constexpr std::array<char, 8> MAGIC_NUMBER = {'S', 'E', 'C', 'L', 'O', 'G', '0', '1'};
    static constexpr uint32_t CURRENT_VERSION = 1;
    static constexpr size_t SALT_SIZE = 16;
    static constexpr size_t IV_SIZE = 12;
    static constexpr size_t AUTH_TAG_SIZE = 16;
};