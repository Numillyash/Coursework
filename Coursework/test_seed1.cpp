#include <iostream>
#include <vector>
#include <random>
#include "lib/bigint/BitBigIntTC.hpp"
#include "tests/legacy_bridge.hpp"

using namespace bigint;
using namespace bigint::test;

std::mt19937_64 rng(0xdeadbeef);

std::vector<uint8_t> random_bits(size_t num_bits, std::mt19937_64& rng) {
    std::uniform_int_distribution<int> bit_dist(0, 1);
    std::vector<uint8_t> bits;
    
    for (size_t i = 0; i < num_bits; ++i) {
        bits.push_back(bit_dist(rng));
    }
    
    std::uniform_int_distribution<int> sign_dist(0, 1);
    bits.push_back(sign_dist(rng));
    
    return bits;
}

int main() {
    for (int seed = 0; seed < 2; ++seed) {
        size_t bits_a = (rng() % 128) + 1;
        size_t bits_b = (rng() % 64) + 1;
        
        auto bits_a_vec = random_bits(bits_a, rng);
        auto bits_b_vec = random_bits(bits_b, rng);
        
        BitBigIntTC tc_a = BitBigIntTC::from_binary_bits(bits_a_vec);
        BitBigIntTC tc_b = BitBigIntTC::from_binary_bits(bits_b_vec);
        
        std::cout << "Seed " << seed << ":" << std::endl;
        std::cout << "  A: " << tc_a.to_binary() << std::endl;
        std::cout << "  B: " << tc_b.to_binary() << std::endl;
        
        if (tc_b.is_zero()) {
            std::cout << "  B is zero, skipping" << std::endl;
            continue;
        }
        
        try {
            auto dm = tc_a.divmod(tc_b);
            std::cout << "  Q: " << dm.q.to_binary() << std::endl;
            std::cout << "  R: " << dm.r.to_binary() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "  Exception: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
