#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "legacy_bridge.hpp"

using namespace bigint;
using namespace bigint::test;

namespace {

std::vector<uint8_t> random_bits(size_t data_bits, std::mt19937_64& rng) {
    std::uniform_int_distribution<int> bit_dist(0, 1);
    std::vector<uint8_t> bits;

    for (size_t i = 0; i < data_bits; ++i)
        bits.push_back(static_cast<uint8_t>(bit_dist(rng)));
    bits.push_back(static_cast<uint8_t>(bit_dist(rng)));
    return bits;
}

number normalized_legacy_from_bits(const std::vector<uint8_t>& bits) {
    number n = legacy_from_bits(bits);
    normalize(&n);
    return n;
}

void fail_case(const std::string& label, const number& legacy_q,
        const number& legacy_r, const DivModTC& tc) {
    std::cerr << "FAIL: " << label << std::endl;
    std::cerr << "  Legacy Q: " << legacy_to_binary(legacy_q) << std::endl;
    std::cerr << "  TC Q:     " << tc_to_binary(tc.q) << std::endl;
    std::cerr << "  Legacy R: " << legacy_to_binary(legacy_r) << std::endl;
    std::cerr << "  TC R:     " << tc_to_binary(tc.r) << std::endl;
    std::exit(1);
}

void run_case(const std::string& label, number& legacy_a, number& legacy_b,
        const BitBigIntTC& tc_a, const BitBigIntTC& tc_b) {
    if (is_zero(&legacy_b)) {
        std::cout << label << " skip-zero-divisor" << std::endl;
        return;
    }

    std::cout << label
              << " dividend=" << legacy_to_binary(legacy_a)
              << " divisor=" << legacy_to_binary(legacy_b)
              << std::endl << std::flush;

    number legacy_r = init();
    number legacy_q = division_with_module(&legacy_a, &legacy_b, &legacy_r);
    normalize(&legacy_q);
    normalize(&legacy_r);

    auto tc_result = tc_a.divmod(tc_b);

    if (legacy_to_binary(legacy_q) != tc_to_binary(tc_result.q)
            || legacy_to_binary(legacy_r) != tc_to_binary(tc_result.r)) {
        fail_case(label, legacy_q, legacy_r, tc_result);
    }

    clear_mem(&legacy_q);
    clear_mem(&legacy_r);
}

void run_int_case(int seed, int index, int dividend, int divisor) {
    number legacy_a = int_to_number(dividend);
    number legacy_b = int_to_number(divisor);
    BitBigIntTC tc_a(static_cast<int64_t>(dividend));
    BitBigIntTC tc_b(static_cast<int64_t>(divisor));

    run_case("seed=" + std::to_string(seed)
            + " index=" + std::to_string(index)
            + " int " + std::to_string(dividend)
            + " / " + std::to_string(divisor),
            legacy_a, legacy_b, tc_a, tc_b);

    clear_mem(&legacy_a);
    clear_mem(&legacy_b);
}

void run_bit_case(int seed, int index, const std::vector<uint8_t>& dividend,
        const std::vector<uint8_t>& divisor) {
    number legacy_a = normalized_legacy_from_bits(dividend);
    number legacy_b = normalized_legacy_from_bits(divisor);
    BitBigIntTC tc_a = BitBigIntTC::from_binary_bits(dividend);
    BitBigIntTC tc_b = BitBigIntTC::from_binary_bits(divisor);

    run_case("seed=" + std::to_string(seed)
            + " index=" + std::to_string(index)
            + " bits dividend_raw=" + bits_to_debug(dividend)
            + " divisor_raw=" + bits_to_debug(divisor),
            legacy_a, legacy_b, tc_a, tc_b);

    clear_mem(&legacy_a);
    clear_mem(&legacy_b);
}

} // namespace

int main() {
    std::vector<std::pair<int, int>> edge_cases = {
        {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {10, 3}, {-10, 3}, {10, -3}, {-10, -3},
        {12, 3}, {-12, 3}, {12, -3}, {-12, -3},
        {3, 10}, {-3, 10}, {3, -10}, {-3, -10},
        {64, 8}, {-64, 8}, {64, -8}, {-64, -8},
        {255, 255}, {-255, -255}, {255, -255}, {-255, 255},
        {1801, 2241}
    };

    int index = 0;
    for (const auto& test : edge_cases)
        run_int_case(0, index++, test.first, test.second);

    const std::vector<uint64_t> seeds = {
        0xdeadbeefULL,
        0xc001d00dULL,
        0x12345678ULL,
        0x5eed5eedULL
    };

    for (uint64_t seed_value : seeds) {
        std::mt19937_64 rng(seed_value);
        std::uniform_int_distribution<int> small_dist(-4096, 4096);
        std::uniform_int_distribution<int> bit_count_dist(1, 128);

        for (int i = 0; i < 250; ++i) {
            int divisor = 0;
            while (divisor == 0)
                divisor = small_dist(rng);
            run_int_case(static_cast<int>(seed_value), i,
                    small_dist(rng), divisor);
        }

        for (int i = 0; i < 250; ++i) {
            auto dividend = random_bits(static_cast<size_t>(bit_count_dist(rng)), rng);
            auto divisor = random_bits(static_cast<size_t>(bit_count_dist(rng)), rng);
            run_bit_case(static_cast<int>(seed_value), i, dividend, divisor);
        }
    }

    std::cout << "divmod stress completed" << std::endl;
    return 0;
}
