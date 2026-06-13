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

bool is_zero_like_bits(const std::vector<uint8_t>& bits) {
    if (bits.size() < 2)
        return true;
    for (size_t i = 0; i + 1 < bits.size(); ++i) {
        if (bits[i] != 0)
            return false;
    }
    return true;
}

bool is_malformed_zero_like_bits(const std::vector<uint8_t>& bits) {
    return is_zero_like_bits(bits) && !bits.empty() && bits.back() != 0;
}

number normalized_legacy_from_bits(const std::vector<uint8_t>& bits) {
    number n = legacy_from_bits(bits);
    normalize(&n);
    return n;
}

std::vector<uint8_t> legacy_raw_bits(const number& value) {
    return std::vector<uint8_t>(value.mas, value.mas + value.current_count);
}

void require_diagnostic(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: malformed divisor diagnostic: " << message << std::endl;
        std::exit(1);
    }
}

void run_malformed_zero_like_diagnostic() {
    const std::vector<uint8_t> divisor = {0, 1};
    number legacy_b = legacy_from_bits(divisor);

    normalize(&legacy_b);
    const auto normalized = legacy_raw_bits(legacy_b);
    const bool legacy_zero = is_zero(&legacy_b);
    const bool malformed_zero_like = is_malformed_zero_like_bits(divisor);

    std::cout << "diagnostic malformed-zero-like divisor_raw="
              << bits_to_debug(divisor)
              << " normalized=" << bits_to_debug(normalized)
              << " legacy_is_zero=" << (legacy_zero ? 1 : 0)
              << " classified_malformed_zero_like="
              << (malformed_zero_like ? 1 : 0)
              << " excluded_from_divmod_oracle"
              << std::endl;

    require_diagnostic(normalized == divisor, "legacy normalize did not preserve [0,1]");
    require_diagnostic(!legacy_zero, "legacy is_zero unexpectedly accepted [0,1]");
    require_diagnostic(malformed_zero_like, "[0,1] was not classified as malformed zero-like");

    clear_mem(&legacy_b);
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
    if (is_malformed_zero_like_bits(dividend)) {
        std::cout << "seed=" << seed
                  << " index=" << index
                  << " bits dividend_raw=" << bits_to_debug(dividend)
                  << " divisor_raw=" << bits_to_debug(divisor)
                  << " skip-malformed-zero-like-dividend"
                  << std::endl;
        return;
    }

    if (is_zero_like_bits(divisor)) {
        std::cout << "seed=" << seed
                  << " index=" << index
                  << " bits dividend_raw=" << bits_to_debug(dividend)
                  << " divisor_raw=" << bits_to_debug(divisor)
                  << (is_malformed_zero_like_bits(divisor)
                      ? " skip-malformed-zero-like-divisor"
                      : " skip-zero-like-divisor")
                  << std::endl;
        return;
    }

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
    run_malformed_zero_like_diagnostic();

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
