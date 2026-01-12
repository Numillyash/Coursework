#include <iostream>
#include "lib/bigint/BitBigIntTC.hpp"

using namespace bigint;

int main() {
    // Simple test: 10 / 3 = 3 remainder 1 (both positive)
    BitBigIntTC a = BitBigIntTC(10);
    BitBigIntTC b = BitBigIntTC(3);
    
    std::cout << "a = " << a.to_binary() << std::endl;
    std::cout << "b = " << b.to_binary() << std::endl;
    
    try {
        auto dm = a.divmod(b);
        std::cout << "Quotient: " << dm.q.to_binary() << std::endl;
        std::cout << "Remainder: " << dm.r.to_binary() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}
