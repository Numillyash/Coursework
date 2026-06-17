#pragma once

#include "BigUint.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace rsa_limb {

using bigint::BigUint;

struct PublicKeyLimb {
    BigUint n;
    BigUint e;
};

struct PrivateKeyLimb {
    BigUint n;
    BigUint d;
};

struct KeyPairLimb {
    PublicKeyLimb public_key;
    PrivateKeyLimb private_key;
    BigUint p;
    BigUint q;
    BigUint phi;
};

BigUint rsa_public_op(const BigUint& message, const PublicKeyLimb& key);
BigUint rsa_private_op(const BigUint& cipher, const PrivateKeyLimb& key);

KeyPairLimb make_keypair_from_primes(
        const BigUint& p,
        const BigUint& q,
        const BigUint& e);

bool rsa_check_keypair(const KeyPairLimb& keypair);

size_t max_plaintext_block_bytes(const PublicKeyLimb& key);
size_t max_plaintext_block_bytes(const PrivateKeyLimb& key);
BigUint bytes_to_block(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> block_to_bytes(
        const BigUint& block,
        size_t byte_count);
std::vector<std::vector<uint8_t>> split_plaintext_blocks(
        const std::vector<uint8_t>& input,
        size_t block_size);
std::vector<BigUint> rsa_encrypt_bytes(
        const std::vector<uint8_t>& input,
        const PublicKeyLimb& key);
std::vector<uint8_t> rsa_decrypt_blocks(
        const std::vector<BigUint>& blocks,
        const PrivateKeyLimb& key,
        size_t original_size);
std::vector<BigUint> rsa_sign_bytes(
        const std::vector<uint8_t>& input,
        const PrivateKeyLimb& key);
std::vector<uint8_t> rsa_recover_signed_bytes(
        const std::vector<BigUint>& signature_blocks,
        const PublicKeyLimb& key,
        size_t original_size);
bool rsa_check_signature_bytes(
        const std::vector<uint8_t>& input,
        const std::vector<BigUint>& signature_blocks,
        const PublicKeyLimb& key);

} // namespace rsa_limb
