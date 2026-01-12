#include <iostream>
#include <vector>
#include "lib/bigint/BitBigIntTC.hpp"

using namespace bigint;

int main() {
    // Test: -10 / 3 (negative dividend)
    // 10 in binary (LSB first) is: 0, 1, 0, 1 (data), 0 (sign=positive)
    // -10 is two's complement: flip all bits and add 1
    // flip: 1, 0, 1, 0, 1
    // add 1: 1, 1, 1, 0, 1 (binary for -10 in two's complement with sign bit at end)
    
    std::vector<uint8_t> neg10_bits = {1, 1, 1, 0, 1};  // -10 in binary (LSB first, sign last)
    BitBigIntTC a = BitBigIntTC::from_binary_bits(neg10_bits);
    
    std::vector<uint8_t> three_bits = {1, 1, 0};  // 3 in binary (LSB first, sign last)
    BitBigIntTC b = BitBigIntTC::from_binary_bits(three_bits);
    
    std::cout << "a = " << a.to_binary() << std::endl;
    std::cout << "b = " << b.to_binary() << std::endl;
    
    try {
        std::cout << "Calling divmod..." << std::endl;
        auto dm = a.divmod(b);
        std::cout << "Quotient: " << dm.q.to_binary() << std::endl;
        std::cout << "Remainder: " << dm.r.to_binary() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}
