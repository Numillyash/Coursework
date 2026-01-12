#include <iostream>
#include <vector>
#include "lib/bigint/BitBigIntTC.hpp"

using namespace bigint;

int main() {
    // Create +10 then convert to -10 and back
    BitBigIntTC a = BitBigIntTC(10);
    std::cout << "+10: " << a.to_binary() << std::endl;
    
    // Now simulate creating -10 from bits
    std::vector<uint8_t> neg10_bits = {0, 1, 1, 0, 1};  // -10 in binary (LSB first, sign last)
    BitBigIntTC neg10 = BitBigIntTC::from_binary_bits(neg10_bits);
    std::cout << "-10 created: " << neg10.to_binary() << std::endl;
    
    // Let's see what the bits look like
    const auto& bits = neg10.raw();
    std::cout << "-10 bits: ";
    for (auto b : bits) std::cout << (int)b;
    std::cout << std::endl;
    
    return 0;
}
