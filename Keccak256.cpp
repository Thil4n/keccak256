
#include <iostream>

using namespace std;

const int HASH_LEN = 32;
const int BLOCK_SIZE = 200 - HASH_LEN * 2;
const int NUM_ROUNDS = 24;

// Static initializers
const unsigned char ROTATION[5][5] = {
    {0, 36, 3, 41, 18},
    {1, 44, 10, 45, 2},
    {62, 6, 43, 15, 61},
    {28, 55, 25, 21, 56},
    {27, 20, 39, 8, 14},
};

static void getHash(const std::uint8_t msg[], std::size_t len, std::uint8_t hashResult[HASH_LEN]);
static void absorb(std::uint64_t state[5][5]);
static std::uint64_t rotl64(std::uint64_t x, int i);

void getHash(const uint8_t msg[], size_t len, uint8_t hashResult[HASH_LEN])
{
    assert((msg != nullptr || len == 0) && hashResult != nullptr);
    uint64_t state[5][5] = {};

    // XOR each message byte into the state, and absorb full blocks
    int blockOff = 0;
    for (size_t i = 0; i < len; i++)
    {
        int j = blockOff >> 3;
        state[j % 5][j / 5] ^= static_cast<uint64_t>(msg[i]) << ((blockOff & 7) << 3);
        blockOff++;
        if (blockOff == BLOCK_SIZE)
        {
            absorb(state);
            blockOff = 0;
        }
    }

    // Final block and padding
    {
        int i = blockOff >> 3;
        state[i % 5][i / 5] ^= UINT64_C(0x01) << ((blockOff & 7) << 3);
        blockOff = BLOCK_SIZE - 1;
        int j = blockOff >> 3;
        state[j % 5][j / 5] ^= UINT64_C(0x80) << ((blockOff & 7) << 3);
        absorb(state);
    }

    // Uint64 array to bytes in little endian
    for (int i = 0; i < HASH_LEN; i++)
    {
        int j = i >> 3;
        hashResult[i] = static_cast<uint8_t>(state[j % 5][j / 5] >> ((i & 7) << 3));
    }
}

void absorb(uint64_t state[5][5])
{
    uint64_t(*a)[5] = state;
    uint8_t r = 1; // LFSR
    for (int i = 0; i < NUM_ROUNDS; i++)
    {
        // Theta step
        uint64_t c[5] = {};
        for (int x = 0; x < 5; x++)
        {
            for (int y = 0; y < 5; y++)
                c[x] ^= a[x][y];
        }
        for (int x = 0; x < 5; x++)
        {
            uint64_t d = c[(x + 4) % 5] ^ rotl64(c[(x + 1) % 5], 1);
            for (int y = 0; y < 5; y++)
                a[x][y] ^= d;
        }

        // Rho and pi steps
        uint64_t b[5][5];
        for (int x = 0; x < 5; x++)
        {
            for (int y = 0; y < 5; y++)
                b[y][(x * 2 + y * 3) % 5] = rotl64(a[x][y], ROTATION[x][y]);
        }

        // Chi step
        for (int x = 0; x < 5; x++)
        {
            for (int y = 0; y < 5; y++)
                a[x][y] = b[x][y] ^ (~b[(x + 1) % 5][y] & b[(x + 2) % 5][y]);
        }

        // Iota step
        for (int j = 0; j < 7; j++)
        {
            a[0][0] ^= static_cast<uint64_t>(r & 1) << ((1 << j) - 1);
            r = static_cast<uint8_t>((r << 1) ^ ((r >> 7) * 0x171));
        }
    }
}

uint64_t rotl64(uint64_t x, int i)
{
    return ((0U + x) << i) | (x >> ((64 - i) & 63));
}

int main()
{
    const char *message = "Hello, World!";
    std::uint8_t hashResult[HASH_LEN];

    getHash(reinterpret_cast<const std::uint8_t *>(message), strlen(message), hashResult);

    std::cout << "Keccak-256 Hash: ";
    for (int i = 0; i < HASH_LEN; i++)
    {
        std::cout << std::hex << static_cast<int>(hashResult[i]);
    }
    std::cout << std::endl;

    return 0;
}