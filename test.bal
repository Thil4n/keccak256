import ballerina/io;

const int HASH_LEN = 32;
const int BLOCK_SIZE = 200 - HASH_LEN * 2;
const int NUM_ROUNDS = 24;

// Static initializers
const int[][] ROTATION = [
    [0, 36, 3, 41, 18],
    [1, 44, 10, 45, 2],
    [62, 6, 43, 15, 61],
    [28, 55, 25, 21, 56],
    [27, 20, 39, 8, 14]
];

function getHash(byte[] msg, int[] hashResult) {

    // Declare a 2D array for the state
    int[5][5] state = [];

    // XOR each message byte into the state, and absorb full blocks
    int blockOff = 0;

    foreach byte b in msg {
        int j = blockOff >> 3;
        state[j % 5][j / 5] = state[j % 5][j / 5] ^ (b << ((blockOff & 7) << 3));
        blockOff += 1;

        if (blockOff == BLOCK_SIZE) {
            absorb(state);
            blockOff = 0;
        }

        io:println(b);
    }

    // Final block and padding
    {
        int i = blockOff >> 3;
        state[i % 5][i / 5] = state[i % 5][i / 5] ^ (1 << ((blockOff & 7) << 3));
        blockOff = BLOCK_SIZE - 1;
        int j = blockOff >> 3;
        state[j % 5][j / 5] = state[j % 5][j / 5] ^ (0x80 << ((blockOff & 7) << 3));
        absorb(state);
    }

    // int array to bytes in little endian
    int j;
    foreach int i in int:range(0, HASH_LEN, 1) {
        j = i >> 3;
        hashResult[i] = state[j % 5][j / 5] >> ((i & 7) << 3);
    }

}

function rotl64(int x, int i) returns int
{
    return ((0 + x) << i) | (x >> ((64 - i) & 63));

}

function absorb(int[][] state) {
    int[][] a = state;
    int r = 1; // LFSR
    int[5] c = [];

    foreach int i in int:range(0, NUM_ROUNDS, 1) {
        // Theta step
        // Initialize c to 0
        c = [0, 0, 0, 0, 0];
        foreach int x in int:range(0, 5, 1) {
            foreach int y in int:range(0, 4, 1) {
                c[x] ^= a[x][y];
            }
        }
        foreach int x in int:range(0, 5, 1) {
            int d = c[(x + 4) % 5] ^ rotl64(c[(x + 1) % 5], 1);
            foreach int y in int:range(0, 5, 1) {
                a[x][y] ^= d;
            }
        }

        // Rho and pi steps
        int[5][5] b = [];
        foreach int x in int:range(0, 5, 1) {
            foreach int y in int:range(0, 5, 1) {
                b[y][(x * 2 + y * 3) % 5] = rotl64(a[x][y], ROTATION[x][y]);
            }
        }

        // Chi step
        foreach int x in int:range(0, 5, 1) {
            foreach int y in int:range(0, 5, 1) {
                a[x][y] = b[x][y] ^ (~b[(x + 1) % 5][y] & b[(x + 2) % 5][y]);
            }
        }

        // Iota step
        foreach int j in int:range(0, 7, 1) {
            a[0][0] ^= (r & 1) << ((1 << j) - 1);
            r = (r << 1) ^ ((r >> 7) * 0x171);
        }
    }
}

public function main() {

    io:println("Block size = ", BLOCK_SIZE);
    io:println("Hash length = ", HASH_LEN);
    io:println("Number of rounds = ", NUM_ROUNDS);

}

