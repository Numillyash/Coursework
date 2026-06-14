#include <algorithm>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>
#include <unistd.h>

#include "legacy_bridge.hpp"

#ifdef max
#undef max
#endif

using namespace bigint;
using namespace bigint::test;

namespace {

std::mt19937_64 rng(0xdeadbeef);
char current_case[256] = "startup";

template <typename T, typename = void>
struct has_mul_method : std::false_type {};

template <typename T>
struct has_mul_method<T,
        std::void_t<decltype(std::declval<T>().mul(std::declval<const T&>()))>>
    : std::true_type {};

template <typename T, typename = void>
struct has_mul_operator : std::false_type {};

template <typename T>
struct has_mul_operator<T,
        std::void_t<decltype(std::declval<T>() * std::declval<T>())>>
    : std::true_type {};

void test_section(const char *name) {
    std::cout << "\n=== " << name << " ===" << std::endl;
}

void set_current_case(const std::string& label) {
    std::snprintf(current_case, sizeof(current_case), "%s", label.c_str());
}

void crash_handler(int signal_number) {
    char buffer[384];
    int len = std::snprintf(buffer, sizeof(buffer),
            "\nSignal %d while running: %s\n", signal_number, current_case);
    if (len > 0)
        write(2, buffer, static_cast<size_t>(len));
    std::_Exit(128 + signal_number);
}

void assert_same_binary(const std::string& label, const number& legacy,
        const BitBigIntTC& tc) {
    std::string legacy_str = legacy_to_binary(legacy);
    std::string tc_str = tc_to_binary(tc);

    if (legacy_str != tc_str) {
        std::cerr << "FAIL: " << label << std::endl;
        std::cerr << "  Legacy: " << legacy_str << std::endl;
        std::cerr << "  TC:     " << tc_str << std::endl;
        std::exit(1);
    }
}

std::vector<int> deterministic_int_cases() {
    std::vector<int> values = {
        0, 1, -1, 2, -2, 3, -3, 5, -5, 7, -7, 10, -10, 31, -31,
        63, -63, 127, -127, 255, -255, 1024, -1024
    };

    for (int n = 0; n <= 20; ++n) {
        int p2 = 1 << n;
        values.push_back(p2);
        values.push_back(-p2);
        values.push_back(p2 - 1);
        values.push_back(-(p2 - 1));
    }
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    return values;
}

std::vector<std::pair<int, int>> deterministic_pairs() {
    std::vector<std::pair<int, int>> pairs = {
        {0, 0}, {0, 1}, {0, -1}, {1, 0}, {-1, 0},
        {1, 1}, {-1, -1}, {1, -1}, {-1, 1},
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {10, 3}, {10, -3}, {-10, 3}, {-10, -3},
        {12, 3}, {-12, 3}, {12, -3}, {-12, -3},
        {3, 10}, {-3, 10}, {3, -10}, {-3, -10},
        {1801, 2241},
        {64, 8}, {-64, 8}, {64, -8}, {-64, -8},
        {127, 7}, {-127, 7}, {127, -7}, {-127, -7},
        {255, 255}, {-255, -255}, {255, -255}, {-255, 255}
    };

    for (int n = 0; n <= 12; ++n) {
        int p2 = 1 << n;
        pairs.push_back({p2, p2});
        pairs.push_back({p2, 1});
        pairs.push_back({p2, -1});
        pairs.push_back({p2 - 1, p2});
        pairs.push_back({-p2, p2});
        pairs.push_back({p2, -p2});
        pairs.push_back({-p2, -p2});
    }
    return pairs;
}

std::vector<uint8_t> random_bits(size_t data_bits, std::mt19937_64& gen) {
    std::uniform_int_distribution<int> bit_dist(0, 1);
    std::vector<uint8_t> bits;

    for (size_t i = 0; i < data_bits; ++i)
        bits.push_back(static_cast<uint8_t>(bit_dist(gen)));
    bits.push_back(static_cast<uint8_t>(bit_dist(gen)));
    return bits;
}

number normalized_legacy_from_bits(const std::vector<uint8_t>& bits) {
    number n = legacy_from_bits(bits);
    normalize(&n);
    return n;
}

std::vector<uint8_t> legacy_raw_bits(const number& n) {
    return std::vector<uint8_t>(n.mas, n.mas + n.current_count);
}

int legacy_compare(number *a, number *b) {
    number diff = difference(a, b);
    normalize(&diff);

    int result = 0;
    if (!is_zero(&diff))
        result = diff.mas[diff.current_count - 1] ? -1 : 1;
    clear_mem(&diff);
    return result;
}

void test_construction_conversion_equivalence() {
    test_section("construction/conversion vs legacy");

    for (int value : deterministic_int_cases()) {
        number legacy_num = int_to_number(value);
        BitBigIntTC tc_num(static_cast<int64_t>(value));
        assert_same_binary("int constructor " + std::to_string(value),
                legacy_num, tc_num);

        BitBigIntTC from_legacy = tc_from_legacy(legacy_num);
        assert_same_binary("tc_from_legacy " + std::to_string(value),
                legacy_num, from_legacy);

        number roundtrip = legacy_from_tc(tc_num);
        normalize(&roundtrip);
        assert_same_binary("legacy_from_tc " + std::to_string(value),
                roundtrip, tc_num);

        clear_mem(&roundtrip);
        clear_mem(&legacy_num);
    }

    std::vector<std::vector<uint8_t>> raw_cases = {
        {0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0, 0}, {1, 0, 0},
        {1, 1, 0}, {0, 1, 1}, {1, 0, 1}, {0, 0, 1, 1},
        {1, 1, 1, 0}, {1, 1, 1, 1}, {0, 1, 0, 1, 1}
    };
    for (const auto& bits : raw_cases) {
        number legacy_num = normalized_legacy_from_bits(bits);
        BitBigIntTC tc_num = BitBigIntTC::from_binary_bits(bits);
        assert_same_binary("raw construction " + bits_to_debug(bits),
                legacy_num, tc_num);
        clear_mem(&legacy_num);
    }

    std::cout << "PASS: construction/conversion deterministic cases" << std::endl;
}

void test_normalize_equivalence() {
    test_section("normalize() vs legacy");

    for (int value : deterministic_int_cases()) {
        number legacy_num = int_to_number(value);
        normalize(&legacy_num);
        BitBigIntTC tc_num(static_cast<int64_t>(value));
        tc_num.normalize();
        assert_same_binary("normalize int " + std::to_string(value),
                legacy_num, tc_num);
        clear_mem(&legacy_num);
    }

    for (int seed = 0; seed < 200; ++seed) {
        auto bits = random_bits((rng() % 80) + 1, rng);
        number legacy_num = normalized_legacy_from_bits(bits);
        BitBigIntTC tc_num = BitBigIntTC::from_binary_bits(bits);
        assert_same_binary("normalize random seed " + std::to_string(seed)
                + " bits=" + bits_to_debug(bits), legacy_num, tc_num);
        clear_mem(&legacy_num);
    }

    std::cout << "PASS: normalize deterministic and random cases" << std::endl;
}

void assert_same_int(const std::string& label, int legacy_value, int tc_value) {
    if (legacy_value != tc_value) {
        std::cerr << "FAIL: " << label << std::endl;
        std::cerr << "  Legacy int: " << legacy_value << std::endl;
        std::cerr << "  TC int:     " << tc_value << std::endl;
        std::exit(1);
    }
}

void test_number_to_int_equivalence() {
    test_section("number_to_int() vs legacy");

    std::vector<int> values = {
        0, 1, -1, 2, -2, 7, -7, 127, -127, 255, -255
    };
    for (int n = 0; n <= 29; ++n) {
        int p2 = 1 << n;
        values.push_back(p2);
        values.push_back(-p2);
        values.push_back(p2 - 1);
        values.push_back(-(p2 - 1));
    }
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());

    for (int value : values) {
        number legacy_num = int_to_number(value);
        BitBigIntTC tc_num(static_cast<int64_t>(value));

        int legacy_value = number_to_int(&legacy_num);
        int tc_value = tc_num.to_int();
        assert_same_int("number_to_int " + std::to_string(value),
                legacy_value, tc_value);

        clear_mem(&legacy_num);
    }

    std::mt19937_64 local_rng(0x51a7e5eedULL);
    std::uniform_int_distribution<int> small_dist(-1000000, 1000000);
    for (int i = 0; i < 500; ++i) {
        int value = small_dist(local_rng);
        number legacy_num = int_to_number(value);
        BitBigIntTC tc_num(static_cast<int64_t>(value));

        int legacy_value = number_to_int(&legacy_num);
        int tc_value = tc_num.to_int();
        assert_same_int("number_to_int random " + std::to_string(i),
                legacy_value, tc_value);

        clear_mem(&legacy_num);
    }

    number legacy_negative = int_to_number(-7);
    const auto before_negative = legacy_raw_bits(legacy_negative);
    (void)number_to_int(&legacy_negative);
    const auto after_negative = legacy_raw_bits(legacy_negative);
    if (before_negative == after_negative) {
        std::cerr << "FAIL: legacy number_to_int did not mutate negative input"
                  << std::endl;
        std::exit(1);
    }
    clear_mem(&legacy_negative);

    number legacy_positive = int_to_number(7);
    const auto before_positive = legacy_raw_bits(legacy_positive);
    (void)number_to_int(&legacy_positive);
    const auto after_positive = legacy_raw_bits(legacy_positive);
    if (before_positive != after_positive) {
        std::cerr << "FAIL: legacy number_to_int mutated positive input"
                  << std::endl;
        std::exit(1);
    }
    clear_mem(&legacy_positive);

    BitBigIntTC tc_negative(static_cast<int64_t>(-7));
    const auto tc_before = tc_negative.raw();
    (void)tc_negative.to_int();
    if (tc_before != tc_negative.raw()) {
        std::cerr << "FAIL: BitBigIntTC::to_int mutated input" << std::endl;
        std::exit(1);
    }

    std::cout << "PASS: number_to_int deterministic and random cases"
              << " (legacy mutates negative inputs; BitBigIntTC::to_int does not)"
              << std::endl;
}

void test_unary_mutation_equivalence() {
    test_section("reverse/add_digit/offset vs legacy");

    for (int value : deterministic_int_cases()) {
        number legacy_rev = int_to_number(value);
        BitBigIntTC tc_rev(static_cast<int64_t>(value));
        reverse(&legacy_rev);
        tc_rev.reverse();
        assert_same_binary("reverse " + std::to_string(value), legacy_rev, tc_rev);
        clear_mem(&legacy_rev);

        for (uint8_t digit : {uint8_t{0}, uint8_t{1}}) {
            number legacy_add = int_to_number(value);
            BitBigIntTC tc_add(static_cast<int64_t>(value));
            add_digit(&legacy_add, digit);
            normalize(&legacy_add);
            tc_add.add_digit(digit);
            tc_add.normalize();
            assert_same_binary("add_digit " + std::to_string(value)
                    + " digit=" + std::to_string(digit), legacy_add, tc_add);
            clear_mem(&legacy_add);
        }

        number legacy_left = int_to_number(value);
        BitBigIntTC tc_left(static_cast<int64_t>(value));
        offset_left(&legacy_left);
        tc_left.offset_left();
        assert_same_binary("offset_left " + std::to_string(value),
                legacy_left, tc_left);
        clear_mem(&legacy_left);

        number legacy_right = int_to_number(value);
        BitBigIntTC tc_right(static_cast<int64_t>(value));
        offset_right(&legacy_right);
        tc_right.offset_right();
        assert_same_binary("offset_right " + std::to_string(value),
                legacy_right, tc_right);
        clear_mem(&legacy_right);
    }

    for (int seed = 0; seed < 100; ++seed) {
        auto bits = random_bits((rng() % 96) + 1, rng);
        number legacy_num = normalized_legacy_from_bits(bits);
        BitBigIntTC tc_num = BitBigIntTC::from_binary_bits(bits);

        offset_left(&legacy_num);
        tc_num.offset_left();
        add_digit(&legacy_num, static_cast<uint8_t>(seed & 1));
        tc_num.add_digit(static_cast<uint8_t>(seed & 1));
        offset_right(&legacy_num);
        tc_num.offset_right();
        reverse(&legacy_num);
        tc_num.reverse();
        normalize(&legacy_num);
        tc_num.normalize();

        assert_same_binary("random unary chain seed " + std::to_string(seed),
                legacy_num, tc_num);
        clear_mem(&legacy_num);
    }

    std::cout << "PASS: reverse/add_digit/offset deterministic and random cases"
              << std::endl;
}

void assert_same_equal(const std::string& label, bool legacy_equal,
        bool tc_equal) {
    if (legacy_equal != tc_equal) {
        std::cerr << "FAIL: " << label << std::endl;
        std::cerr << "  Legacy equal: " << legacy_equal << std::endl;
        std::cerr << "  TC equal:     " << tc_equal << std::endl;
        std::exit(1);
    }
}

void test_is_equal_equivalence() {
    test_section("is_equal() vs legacy");

    std::vector<std::pair<int, int>> pairs = {
        {7, 7}, {7, 8},
        {-7, -7}, {-7, -8},
        {0, 0}, {0, -7},
        {127, 127}, {127, -127},
        {-255, -255}, {-255, 255}
    };

    std::mt19937_64 local_rng(0xe9a1ULL);
    std::uniform_int_distribution<int> small_dist(-4096, 4096);
    for (int i = 0; i < 300; ++i)
        pairs.push_back({small_dist(local_rng), small_dist(local_rng)});

    for (const auto& pair : pairs) {
        number legacy_a = int_to_number(pair.first);
        number legacy_b = int_to_number(pair.second);
        BitBigIntTC tc_a(static_cast<int64_t>(pair.first));
        BitBigIntTC tc_b(static_cast<int64_t>(pair.second));

        bool legacy_equal = is_equal(&legacy_a, &legacy_b);
        bool tc_equal = tc_a.is_equal(tc_b);
        assert_same_equal("is_equal " + std::to_string(pair.first)
                + " vs " + std::to_string(pair.second),
                legacy_equal, tc_equal);

        clear_mem(&legacy_a);
        clear_mem(&legacy_b);
    }

    std::vector<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> raw_pairs = {
        {{0, 0, 0}, {0, 0}},
        {{1, 0, 0}, {1, 0}},
        {{0, 1, 0, 0}, {0, 1, 0}},
        {{1, 1, 0, 0}, {1, 1, 0}},
        {{0, 0, 0, 0, 0}, {0, 0}}
    };

    for (const auto& pair : raw_pairs) {
        number legacy_a = legacy_from_bits(pair.first);
        number legacy_b = legacy_from_bits(pair.second);
        BitBigIntTC tc_a = BitBigIntTC::from_binary_bits(pair.first);
        BitBigIntTC tc_b = BitBigIntTC::from_binary_bits(pair.second);

        bool legacy_equal = is_equal(&legacy_a, &legacy_b);
        bool tc_equal = tc_a.is_equal(tc_b);
        assert_same_equal("is_equal raw " + bits_to_debug(pair.first)
                + " vs " + bits_to_debug(pair.second),
                legacy_equal, tc_equal);

        clear_mem(&legacy_a);
        clear_mem(&legacy_b);
    }

    std::cout << "PASS: is_equal deterministic and random cases" << std::endl;
}

void test_comparison_equivalence() {
    test_section("comparison vs legacy");

    auto pairs = deterministic_pairs();
    for (const auto& pair : pairs) {
        number legacy_a = int_to_number(pair.first);
        number legacy_b = int_to_number(pair.second);
        BitBigIntTC tc_a(static_cast<int64_t>(pair.first));
        BitBigIntTC tc_b(static_cast<int64_t>(pair.second));

        number legacy_a_for_equal = copy(&legacy_a);
        number legacy_b_for_equal = copy(&legacy_b);
        bool legacy_equal = is_equal(&legacy_a_for_equal, &legacy_b_for_equal);
        bool tc_equal = (tc_a == tc_b);
        if (legacy_equal != tc_equal) {
            std::cerr << "FAIL: equality " << pair.first << " vs "
                      << pair.second << std::endl;
            std::cerr << "  Legacy equal: " << legacy_equal << std::endl;
            std::cerr << "  TC equal:     " << tc_equal << std::endl;
            std::exit(1);
        }
        clear_mem(&legacy_a_for_equal);
        clear_mem(&legacy_b_for_equal);

        number legacy_a_for_cmp = copy(&legacy_a);
        number legacy_b_for_cmp = copy(&legacy_b);
        int legacy_cmp = legacy_compare(&legacy_a_for_cmp, &legacy_b_for_cmp);
        int tc_cmp = tc_a.compare(tc_b);
        tc_cmp = (tc_cmp > 0) - (tc_cmp < 0);
        if (legacy_cmp != tc_cmp) {
            std::cerr << "FAIL: compare " << pair.first << " vs "
                      << pair.second << std::endl;
            std::cerr << "  Legacy compare: " << legacy_cmp << std::endl;
            std::cerr << "  TC compare:     " << tc_cmp << std::endl;
            std::cerr << "  A legacy: " << legacy_to_binary(legacy_a) << std::endl;
            std::cerr << "  B legacy: " << legacy_to_binary(legacy_b) << std::endl;
            std::exit(1);
        }
        clear_mem(&legacy_a_for_cmp);
        clear_mem(&legacy_b_for_cmp);
        clear_mem(&legacy_a);
        clear_mem(&legacy_b);
    }

    std::cout << "PASS: comparison deterministic cases" << std::endl;
}

void test_addition_difference_equivalence() {
    test_section("addition/difference vs legacy");

    auto pairs = deterministic_pairs();
    std::uniform_int_distribution<int> small_dist(-4096, 4096);
    std::uniform_int_distribution<int> int32_dist(
            std::numeric_limits<int>::min() + 1,
            std::numeric_limits<int>::max());

    for (int i = 0; i < 200; ++i)
        pairs.push_back({small_dist(rng), small_dist(rng)});
    for (int i = 0; i < 100; ++i)
        pairs.push_back({int32_dist(rng), int32_dist(rng)});

    for (const auto& pair : pairs) {
        number legacy_a = int_to_number(pair.first);
        number legacy_b = int_to_number(pair.second);
        BitBigIntTC tc_a(static_cast<int64_t>(pair.first));
        BitBigIntTC tc_b(static_cast<int64_t>(pair.second));

        number legacy_sum = addition(&legacy_a, &legacy_b);
        BitBigIntTC tc_sum = tc_a.add(tc_b);
        assert_same_binary("addition " + std::to_string(pair.first) + " + "
                + std::to_string(pair.second), legacy_sum, tc_sum);
        clear_mem(&legacy_sum);

        number legacy_diff = difference(&legacy_a, &legacy_b);
        normalize(&legacy_diff);
        BitBigIntTC tc_diff = tc_a.sub(tc_b);
        tc_diff.normalize();
        assert_same_binary("difference " + std::to_string(pair.first) + " - "
                + std::to_string(pair.second), legacy_diff, tc_diff);
        clear_mem(&legacy_diff);

        clear_mem(&legacy_a);
        clear_mem(&legacy_b);
    }

    for (int seed = 0; seed < 100; ++seed) {
        auto bits_a = random_bits((rng() % 160) + 1, rng);
        auto bits_b = random_bits((rng() % 160) + 1, rng);
        number legacy_a = normalized_legacy_from_bits(bits_a);
        number legacy_b = normalized_legacy_from_bits(bits_b);
        BitBigIntTC tc_a = BitBigIntTC::from_binary_bits(bits_a);
        BitBigIntTC tc_b = BitBigIntTC::from_binary_bits(bits_b);

        number legacy_sum = addition(&legacy_a, &legacy_b);
        BitBigIntTC tc_sum = tc_a.add(tc_b);
        assert_same_binary("random-bit addition seed " + std::to_string(seed),
                legacy_sum, tc_sum);
        clear_mem(&legacy_sum);

        number legacy_diff = difference(&legacy_a, &legacy_b);
        normalize(&legacy_diff);
        BitBigIntTC tc_diff = tc_a.sub(tc_b);
        tc_diff.normalize();
        assert_same_binary("random-bit difference seed " + std::to_string(seed),
                legacy_diff, tc_diff);
        clear_mem(&legacy_diff);

        clear_mem(&legacy_a);
        clear_mem(&legacy_b);
    }

    std::cout << "PASS: addition/difference deterministic and random cases"
              << std::endl;
}

template <typename T>
void test_multiplication_for_type() {
    if constexpr (!has_mul_method<T>::value && !has_mul_operator<T>::value) {
        std::cout << "SKIP: BitBigIntTC has no multiplication API yet"
                  << std::endl;
        return;
    } else {
        for (const auto& pair : deterministic_pairs()) {
            number legacy_a = int_to_number(pair.first);
            number legacy_b = int_to_number(pair.second);
            number legacy_product = multiplication(&legacy_a, &legacy_b);
            T tc_a(static_cast<int64_t>(pair.first));
            T tc_b(static_cast<int64_t>(pair.second));

            if constexpr (has_mul_method<T>::value) {
                T tc_product = tc_a.mul(tc_b);
                assert_same_binary("multiplication " + std::to_string(pair.first)
                        + " * " + std::to_string(pair.second),
                        legacy_product, tc_product);
            } else if constexpr (has_mul_operator<T>::value) {
                T tc_product = tc_a * tc_b;
                assert_same_binary("multiplication " + std::to_string(pair.first)
                        + " * " + std::to_string(pair.second),
                        legacy_product, tc_product);
            }

            clear_mem(&legacy_product);
            clear_mem(&legacy_a);
            clear_mem(&legacy_b);
        }
        std::cout << "PASS: multiplication deterministic cases" << std::endl;
    }
}

void test_multiplication_equivalence_if_available() {
    test_section("multiplication vs legacy");
    test_multiplication_for_type<BitBigIntTC>();
}

void test_divmod_equivalence() {
    test_section("divmod/modulo vs legacy");

    std::vector<std::pair<int, int>> pairs = {
        {1801, 2241},
        {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {10, 3}, {10, -3}, {-10, 3}, {-10, -3},
        {12, 3}, {-12, 3}, {12, -3}, {-12, -3},
        {3, 10}, {-3, 10}, {3, -10}, {-3, -10},
        {64, 8}, {-64, 8}, {64, -8}, {-64, -8},
        {255, 255}, {-255, -255}, {255, -255}, {-255, 255}
    };

    std::uniform_int_distribution<int> small_dist(-4096, 4096);
    for (int i = 0; i < 5; ++i) {
        int divisor = 0;
        while (divisor == 0)
            divisor = small_dist(rng);
        pairs.push_back({small_dist(rng), divisor});
    }

    for (const auto& pair : pairs) {
        set_current_case("divmod int " + std::to_string(pair.first) + " / "
                + std::to_string(pair.second));
        number legacy_a = int_to_number(pair.first);
        number legacy_b = int_to_number(pair.second);
        number legacy_r = init();
        number legacy_q = division_with_module(&legacy_a, &legacy_b, &legacy_r);
        normalize(&legacy_q);
        normalize(&legacy_r);

        BitBigIntTC tc_a(static_cast<int64_t>(pair.first));
        BitBigIntTC tc_b(static_cast<int64_t>(pair.second));
        auto dm = tc_a.divmod(tc_b);

        assert_same_binary("divmod quotient " + std::to_string(pair.first)
                + " / " + std::to_string(pair.second), legacy_q, dm.q);
        assert_same_binary("divmod remainder/modulo " + std::to_string(pair.first)
                + " % " + std::to_string(pair.second), legacy_r, dm.r);

        clear_mem(&legacy_q);
        clear_mem(&legacy_r);
        clear_mem(&legacy_a);
        clear_mem(&legacy_b);
    }

    for (int seed = 0; seed < 5; ++seed) {
        set_current_case("divmod random-bit seed " + std::to_string(seed));
        auto bits_a = random_bits((rng() % 64) + 1, rng);
        auto bits_b = random_bits((rng() % 32) + 1, rng);
        number legacy_a = normalized_legacy_from_bits(bits_a);
        number legacy_b = normalized_legacy_from_bits(bits_b);
        if (is_zero(&legacy_b)) {
            clear_mem(&legacy_a);
            clear_mem(&legacy_b);
            continue;
        }

        number legacy_r = init();
        number legacy_q = division_with_module(&legacy_a, &legacy_b, &legacy_r);
        normalize(&legacy_q);
        normalize(&legacy_r);

        BitBigIntTC tc_a = BitBigIntTC::from_binary_bits(bits_a);
        BitBigIntTC tc_b = BitBigIntTC::from_binary_bits(bits_b);
        auto dm = tc_a.divmod(tc_b);

        assert_same_binary("random-bit divmod quotient seed "
                + std::to_string(seed), legacy_q, dm.q);
        assert_same_binary("random-bit divmod remainder seed "
                + std::to_string(seed), legacy_r, dm.r);

        clear_mem(&legacy_q);
        clear_mem(&legacy_r);
        clear_mem(&legacy_a);
        clear_mem(&legacy_b);
    }

    std::cout << "PASS: divmod/modulo deterministic and random cases"
              << std::endl;
}

} // namespace

int main() {
    std::signal(SIGABRT, crash_handler);
    std::signal(SIGBUS, crash_handler);
    std::signal(SIGFPE, crash_handler);
    std::signal(SIGILL, crash_handler);
    std::signal(SIGSEGV, crash_handler);

    std::cout << "BitBigIntTC vs Legacy Tests" << std::endl;
    std::cout << "===========================\n" << std::endl;

    test_construction_conversion_equivalence();
    test_normalize_equivalence();
    test_number_to_int_equivalence();
    test_unary_mutation_equivalence();
    test_is_equal_equivalence();
    test_addition_difference_equivalence();
    test_multiplication_equivalence_if_available();
    test_divmod_equivalence();
    test_comparison_equivalence();

    std::cout << "\n============================================" << std::endl;
    std::cout << "All tests PASSED!" << std::endl;
    std::cout << "============================================" << std::endl;
    return 0;
}
