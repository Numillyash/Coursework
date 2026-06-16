#include "RSA_Limb.hpp"

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

} // namespace

int main() {
    std::cout << "RSA_Limb BigUint Smoke Tests\n";
    std::cout << "============================" << std::endl;

    test_classic_small_rsa();
    test_65537_roundtrips();
    test_key_validation_errors();
    test_operation_validation_errors();
    test_additional_small_prime_pairs();

    std::cout << "\nRSA_Limb smoke tests PASSED" << std::endl;
    return 0;
}
