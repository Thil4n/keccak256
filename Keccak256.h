
#pragma once

#include <cstddef>
#include <cstdint>

class Keccak256 final
{

public:
    static const int HASH_LEN = 32;

private:
    static const int BLOCK_SIZE = 200 - HASH_LEN * 2;

private:
    static const int NUM_ROUNDS = 24;

public:
    static void getHash(const std::uint8_t msg[], std::size_t len, std::uint8_t hashResult[HASH_LEN]);

private:
    static void absorb(std::uint64_t state[5][5]);

    // Requires 0 <= i <= 63
private:
    static std::uint64_t rotl64(std::uint64_t x, int i);

    Keccak256() = delete; // Not instantiable

private:
    static const unsigned char ROTATION[5][5];
};