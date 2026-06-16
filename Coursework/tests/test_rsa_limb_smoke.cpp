#include "RSA_Limb.hpp"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using bigint::BigUint;
using rsa_limb::KeyPairLimb;
using rsa_limb::PrivateKeyLimb;
using rsa_limb::PublicKeyLimb;

namespace {

[[noreturn]] void fail(const std::string& message) {
    std::cerr << "FAIL: " << message << std::endl;
    std::exit(1);
}

void expect(bool condition, const std::string& message) {
    if (!condition)
        fail(message);
}

void expect_eq_u64(
        const BigUint& actual,
        uint64_t expected,
        const std::string& message) {
    if (actual.to_uint64_for_testing() != expected) {
        std::cerr << "FAIL: " << message << std::endl;
        std::cerr << "  actual:   " << actual.to_uint64_for_testing()
                  << std::endl;
        std::cerr << "  expected: " << expected << std::endl;
        std::exit(1);
    }
}

void expect_bytes_eq(
        const std::vector<uint8_t>& actual,
        const std::vector<uint8_t>& expected,
        const std::string& message) {
    if (actual != expected) {
        std::cerr << "FAIL: " << message << std::endl;
        std::cerr << "  actual size:   " << actual.size() << std::endl;
        std::cerr << "  expected size: " << expected.size() << std::endl;
        std::exit(1);
    }
}

template <typename Fn>
void expect_invalid_argument(Fn fn, const std::string& message) {
    bool threw = false;
    try {
        fn();
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    expect(threw, message);
}

void expect_roundtrip(const KeyPairLimb& keypair, const BigUint& message) {
    BigUint cipher = rsa_limb::rsa_public_op(message, keypair.public_key);
    BigUint plain = rsa_limb::rsa_private_op(cipher, keypair.private_key);
    expect(plain == message, "RSA_Limb encrypt/decrypt roundtrip");
}

void expect_byte_roundtrip(
        const KeyPairLimb& keypair,
        const std::vector<uint8_t>& input) {
    std::vector<BigUint> encrypted =
            rsa_limb::rsa_encrypt_bytes(input, keypair.public_key);
    std::vector<uint8_t> decrypted = rsa_limb::rsa_decrypt_blocks(
            encrypted, keypair.private_key, input.size());
    expect_bytes_eq(decrypted, input, "RSA_Limb byte roundtrip");
}

void test_encoding_helpers() {
    std::cout << "\n=== byte/block encoding helpers ===" << std::endl;

    expect(rsa_limb::bytes_to_block({}).is_zero(),
            "empty bytes encode as zero");
    expect_eq_u64(rsa_limb::bytes_to_block({0x34, 0x12}), 0x1234,
            "little-endian bytes_to_block");
    expect_bytes_eq(rsa_limb::block_to_bytes(BigUint(0x1234), 2),
            {0x34, 0x12}, "little-endian block_to_bytes");
    expect_bytes_eq(rsa_limb::block_to_bytes(BigUint(0), 3),
            {0, 0, 0}, "block_to_bytes zero padding");
    expect_invalid_argument([] {
        (void)rsa_limb::block_to_bytes(BigUint(0x1234), 1);
    }, "block_to_bytes too-small count throws");
    expect_invalid_argument([] {
        (void)rsa_limb::block_to_bytes(BigUint(1), 0);
    }, "nonzero block_to_bytes zero count throws");
    expect_bytes_eq(rsa_limb::block_to_bytes(BigUint(0), 0),
            {}, "zero block_to_bytes zero count");
}

void test_split_plaintext_blocks() {
    std::cout << "\n=== split plaintext blocks ===" << std::endl;

    expect(rsa_limb::split_plaintext_blocks({}, 3).empty(),
            "empty input splits to empty blocks");
    expect_invalid_argument([] {
        (void)rsa_limb::split_plaintext_blocks({1, 2, 3}, 0);
    }, "zero block size throws");

    std::vector<std::vector<uint8_t>> exact =
            rsa_limb::split_plaintext_blocks({1, 2}, 2);
    expect(exact.size() == 1, "exact block count");
    expect_bytes_eq(exact[0], {1, 2}, "exact block bytes");

    std::vector<std::vector<uint8_t>> full =
            rsa_limb::split_plaintext_blocks({1, 2, 3, 4}, 2);
    expect(full.size() == 2, "multiple full block count");
    expect_bytes_eq(full[0], {1, 2}, "first full block");
    expect_bytes_eq(full[1], {3, 4}, "second full block");

    std::vector<std::vector<uint8_t>> partial =
            rsa_limb::split_plaintext_blocks({1, 2, 3, 4, 5}, 2);
    expect(partial.size() == 3, "partial block count");
    expect_bytes_eq(partial[2], {5}, "final partial block");
}

void test_max_plaintext_block_bytes() {
    std::cout << "\n=== max plaintext block bytes ===" << std::endl;

    PublicKeyLimb classic{BigUint(3233), BigUint(17)};
    expect(rsa_limb::max_plaintext_block_bytes(classic) == 1,
            "classic n=3233 has one-byte blocks");
    expect_invalid_argument([] {
        (void)rsa_limb::max_plaintext_block_bytes(
                PublicKeyLimb{BigUint(0), BigUint(17)});
    }, "zero modulus block size throws");
    expect_invalid_argument([] {
        (void)rsa_limb::max_plaintext_block_bytes(
                PublicKeyLimb{BigUint(2), BigUint(17)});
    }, "tiny modulus block size throws");
}

void test_classic_small_rsa() {
    std::cout << "\n=== classic small RSA vector ===" << std::endl;

    KeyPairLimb keypair = rsa_limb::make_keypair_from_primes(
            BigUint(61), BigUint(53), BigUint(17));
    expect_eq_u64(keypair.public_key.n, 3233, "classic n");
    expect_eq_u64(keypair.phi, 3120, "classic phi");
    expect_eq_u64(keypair.private_key.d, 2753, "classic d");
    expect(rsa_limb::rsa_check_keypair(keypair), "classic keypair check");

    BigUint cipher = rsa_limb::rsa_public_op(BigUint(65), keypair.public_key);
    expect_eq_u64(cipher, 2790, "classic cipher");
    expect_eq_u64(rsa_limb::rsa_private_op(cipher, keypair.private_key),
            65, "classic decrypt");
}

void test_65537_roundtrips() {
    std::cout << "\n=== small RSA with e=65537 ===" << std::endl;

    KeyPairLimb keypair = rsa_limb::make_keypair_from_primes(
            BigUint(61), BigUint(53), BigUint(65537));
    expect(rsa_limb::rsa_check_keypair(keypair), "e=65537 keypair check");
    expect(BigUint::mod_mul(keypair.public_key.e, keypair.private_key.d,
                   keypair.phi) == BigUint::one(),
            "e*d mod phi == 1");

    const std::vector<BigUint> messages = {
        BigUint(0),
        BigUint(1),
        BigUint(2),
        BigUint(42),
        BigUint(65),
        keypair.public_key.n.sub_abs(BigUint::one()),
    };
    for (const BigUint& message : messages)
        expect_roundtrip(keypair, message);
}

void test_key_validation_errors() {
    std::cout << "\n=== key validation errors ===" << std::endl;

    expect_invalid_argument([] {
        (void)rsa_limb::make_keypair_from_primes(
                BigUint(61), BigUint(61), BigUint(17));
    }, "p == q throws");
    expect_invalid_argument([] {
        (void)rsa_limb::make_keypair_from_primes(
                BigUint(1), BigUint(53), BigUint(17));
    }, "p <= 1 throws");
    expect_invalid_argument([] {
        (void)rsa_limb::make_keypair_from_primes(
                BigUint(61), BigUint(1), BigUint(17));
    }, "q <= 1 throws");
    expect_invalid_argument([] {
        (void)rsa_limb::make_keypair_from_primes(
                BigUint(61), BigUint(53), BigUint(0));
    }, "e == 0 throws");
    expect_invalid_argument([] {
        (void)rsa_limb::make_keypair_from_primes(
                BigUint(5), BigUint(11), BigUint(10));
    }, "non-coprime e/phi throws");
}

void test_operation_validation_errors() {
    std::cout << "\n=== operation validation errors ===" << std::endl;

    expect_invalid_argument([] {
        (void)rsa_limb::rsa_public_op(
                BigUint(1), PublicKeyLimb{BigUint(0), BigUint(3)});
    }, "public op n == 0 throws");
    expect_invalid_argument([] {
        (void)rsa_limb::rsa_public_op(
                BigUint(1), PublicKeyLimb{BigUint(10), BigUint(0)});
    }, "public op e == 0 throws");
    expect_invalid_argument([] {
        (void)rsa_limb::rsa_private_op(
                BigUint(1), PrivateKeyLimb{BigUint(0), BigUint(3)});
    }, "private op n == 0 throws");
    expect_invalid_argument([] {
        (void)rsa_limb::rsa_private_op(
                BigUint(1), PrivateKeyLimb{BigUint(10), BigUint(0)});
    }, "private op d == 0 throws");
    expect_invalid_argument([] {
        (void)rsa_limb::rsa_public_op(
                BigUint(10), PublicKeyLimb{BigUint(10), BigUint(3)});
    }, "message >= n throws");
    expect_invalid_argument([] {
        (void)rsa_limb::rsa_private_op(
                BigUint(10), PrivateKeyLimb{BigUint(10), BigUint(3)});
    }, "cipher >= n throws");
}

void test_additional_small_prime_pairs() {
    std::cout << "\n=== additional deterministic small prime pairs ==="
              << std::endl;

    struct Case {
        uint64_t p;
        uint64_t q;
        uint64_t e;
    };
    const std::vector<Case> cases = {
        {47, 59, 17},
        {71, 67, 79},
        {83, 89, 65537},
    };

    for (const Case& test : cases) {
        KeyPairLimb keypair = rsa_limb::make_keypair_from_primes(
                BigUint(test.p), BigUint(test.q), BigUint(test.e));
        expect(rsa_limb::rsa_check_keypair(keypair),
                "deterministic keypair check");

        const std::vector<BigUint> messages = {
            BigUint(0),
            BigUint(1),
            BigUint(2),
            BigUint(42),
            keypair.public_key.n.sub_abs(BigUint::one()),
        };
        for (const BigUint& message : messages)
            expect_roundtrip(keypair, message);
    }
}

void test_byte_roundtrips() {
    std::cout << "\n=== RSA_Limb byte roundtrips ===" << std::endl;

    KeyPairLimb classic = rsa_limb::make_keypair_from_primes(
            BigUint(61), BigUint(53), BigUint(17));
    expect(rsa_limb::max_plaintext_block_bytes(classic.public_key) == 1,
            "classic byte block size");

    expect_byte_roundtrip(classic, {});
    expect_byte_roundtrip(classic, {65});
    expect_byte_roundtrip(classic, {0});
    expect_byte_roundtrip(classic, {0, 0, 65});
    expect_byte_roundtrip(classic, {1, 2, 3, 4, 5});

    KeyPairLimb two_byte_key = rsa_limb::make_keypair_from_primes(
            BigUint(257), BigUint(263), BigUint(17));
    expect(rsa_limb::max_plaintext_block_bytes(two_byte_key.public_key) == 2,
            "two-byte block size");
    expect_byte_roundtrip(two_byte_key, {0x34, 0x12});
    expect_byte_roundtrip(two_byte_key, {0, 0});
    expect_byte_roundtrip(two_byte_key, {1, 2, 3, 4});
    expect_byte_roundtrip(two_byte_key, {1, 2, 3});

    std::vector<uint8_t> all_bytes;
    for (int i = 0; i <= 255; ++i)
        all_bytes.push_back(static_cast<uint8_t>(i));
    expect_byte_roundtrip(two_byte_key, all_bytes);
}

void test_decrypt_validation() {
    std::cout << "\n=== RSA_Limb byte decrypt validation ===" << std::endl;

    KeyPairLimb keypair = rsa_limb::make_keypair_from_primes(
            BigUint(257), BigUint(263), BigUint(17));
    std::vector<BigUint> encrypted =
            rsa_limb::rsa_encrypt_bytes({1, 2, 3}, keypair.public_key);

    expect_invalid_argument([&] {
        (void)rsa_limb::rsa_decrypt_blocks(
                {BigUint(0)}, keypair.private_key, 0);
    }, "nonempty blocks for zero original size throws");
    expect_invalid_argument([&] {
        (void)rsa_limb::rsa_decrypt_blocks({}, keypair.private_key, 1);
    }, "empty blocks for nonzero original size throws");
    expect_invalid_argument([&] {
        (void)rsa_limb::rsa_decrypt_blocks(
                {encrypted[0]}, keypair.private_key, 3);
    }, "block count mismatch throws");
    expect_invalid_argument([&] {
        (void)rsa_limb::rsa_decrypt_blocks(
                {keypair.private_key.n}, keypair.private_key, 1);
    }, "cipher block >= n throws");
}

} // namespace

int main() {
    std::cout << "RSA_Limb BigUint Smoke Tests\n";
    std::cout << "============================" << std::endl;

    test_encoding_helpers();
    test_split_plaintext_blocks();
    test_max_plaintext_block_bytes();
    test_classic_small_rsa();
    test_65537_roundtrips();
    test_key_validation_errors();
    test_operation_validation_errors();
    test_additional_small_prime_pairs();
    test_byte_roundtrips();
    test_decrypt_validation();

    std::cout << "\nRSA_Limb smoke tests PASSED" << std::endl;
    return 0;
}
