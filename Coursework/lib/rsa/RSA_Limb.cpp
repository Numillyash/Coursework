#include "RSA_Limb.hpp"

#include <stdexcept>

namespace rsa_limb {

namespace {

size_t max_plaintext_block_bytes_for_modulus(const BigUint& n) {
    if (n.is_zero())
        throw std::invalid_argument("RSA_Limb zero modulus");

    size_t result = (n.bit_length() - 1) / 8;
    if (result == 0)
        throw std::invalid_argument("RSA_Limb modulus too small for bytes");
    return result;
}

void validate_public_key(const PublicKeyLimb& key) {
    if (key.n.is_zero())
        throw std::invalid_argument("rsa_public_op zero modulus");
    if (key.e.is_zero())
        throw std::invalid_argument("rsa_public_op zero exponent");
}

void validate_private_key(const PrivateKeyLimb& key) {
    if (key.n.is_zero())
        throw std::invalid_argument("rsa_private_op zero modulus");
    if (key.d.is_zero())
        throw std::invalid_argument("rsa_private_op zero exponent");
}

} // namespace

BigUint rsa_public_op(const BigUint& message, const PublicKeyLimb& key) {
    validate_public_key(key);
    if (message.compare(key.n) >= 0)
        throw std::invalid_argument("rsa_public_op message out of range");
    return BigUint::mod_pow(message, key.e, key.n);
}

BigUint rsa_private_op(const BigUint& cipher, const PrivateKeyLimb& key) {
    validate_private_key(key);
    if (cipher.compare(key.n) >= 0)
        throw std::invalid_argument("rsa_private_op cipher out of range");
    return BigUint::mod_pow(cipher, key.d, key.n);
}

KeyPairLimb make_keypair_from_primes(
        const BigUint& p,
        const BigUint& q,
        const BigUint& e) {
    if (p.compare(BigUint::one()) <= 0)
        throw std::invalid_argument("make_keypair_from_primes p <= 1");
    if (q.compare(BigUint::one()) <= 0)
        throw std::invalid_argument("make_keypair_from_primes q <= 1");
    if (p == q)
        throw std::invalid_argument("make_keypair_from_primes p == q");
    if (e.is_zero())
        throw std::invalid_argument("make_keypair_from_primes zero e");

    BigUint n = p.mul_schoolbook(q);
    BigUint phi = p.sub_abs(BigUint::one()).mul_schoolbook(
            q.sub_abs(BigUint::one()));
    if (BigUint::gcd(e, phi) != BigUint::one())
        throw std::invalid_argument("make_keypair_from_primes e not coprime");

    BigUint d = BigUint::mod_inverse(e, phi);
    return KeyPairLimb{
        PublicKeyLimb{n, e},
        PrivateKeyLimb{n, d},
        p,
        q,
        phi,
    };
}

bool rsa_check_keypair(const KeyPairLimb& keypair) {
    if (keypair.public_key.n != keypair.private_key.n)
        return false;
    if (keypair.public_key.n != keypair.p.mul_schoolbook(keypair.q))
        return false;
    BigUint check = BigUint::mod_mul(
            keypair.public_key.e,
            keypair.private_key.d,
            keypair.phi);
    return check == BigUint::one();
}

size_t max_plaintext_block_bytes(const PublicKeyLimb& key) {
    return max_plaintext_block_bytes_for_modulus(key.n);
}

BigUint bytes_to_block(const std::vector<uint8_t>& bytes) {
    BigUint result;
    for (size_t byte = 0; byte < bytes.size(); ++byte) {
        for (size_t bit = 0; bit < 8; ++bit) {
            if (((bytes[byte] >> bit) & 1U) != 0)
                result.set_bit(byte * 8 + bit);
        }
    }
    return result;
}

std::vector<uint8_t> block_to_bytes(
        const BigUint& block,
        size_t byte_count) {
    if (byte_count == 0) {
        if (!block.is_zero())
            throw std::invalid_argument("block does not fit byte_count");
        return {};
    }

    if (block.bit_length() > byte_count * 8)
        throw std::invalid_argument("block does not fit byte_count");

    std::vector<uint8_t> bytes(byte_count, 0);
    for (size_t byte = 0; byte < byte_count; ++byte) {
        uint8_t value = 0;
        for (size_t bit = 0; bit < 8; ++bit) {
            if (block.test_bit(byte * 8 + bit))
                value = static_cast<uint8_t>(value | (uint8_t{1} << bit));
        }
        bytes[byte] = value;
    }
    return bytes;
}

std::vector<std::vector<uint8_t>> split_plaintext_blocks(
        const std::vector<uint8_t>& input,
        size_t block_size) {
    if (block_size == 0)
        throw std::invalid_argument("split_plaintext_blocks zero block size");

    std::vector<std::vector<uint8_t>> blocks;
    for (size_t offset = 0; offset < input.size(); offset += block_size) {
        size_t end = offset + block_size;
        if (end > input.size())
            end = input.size();
        blocks.emplace_back(input.begin() + static_cast<std::ptrdiff_t>(offset),
                input.begin() + static_cast<std::ptrdiff_t>(end));
    }
    return blocks;
}

std::vector<BigUint> rsa_encrypt_bytes(
        const std::vector<uint8_t>& input,
        const PublicKeyLimb& key) {
    size_t block_size = max_plaintext_block_bytes(key);
    std::vector<BigUint> encrypted;
    for (const std::vector<uint8_t>& block
            : split_plaintext_blocks(input, block_size)) {
        BigUint plaintext = bytes_to_block(block);
        if (plaintext.compare(key.n) >= 0)
            throw std::invalid_argument("plaintext block out of range");
        encrypted.push_back(rsa_public_op(plaintext, key));
    }
    return encrypted;
}

std::vector<uint8_t> rsa_decrypt_blocks(
        const std::vector<BigUint>& blocks,
        const PrivateKeyLimb& key,
        size_t original_size) {
    size_t block_size = max_plaintext_block_bytes_for_modulus(key.n);
    if (original_size == 0 && !blocks.empty())
        throw std::invalid_argument("nonempty blocks for empty output");
    if (original_size > 0 && blocks.empty())
        throw std::invalid_argument("empty blocks for nonempty output");

    size_t expected_blocks =
            original_size == 0 ? 0 : (original_size + block_size - 1) / block_size;
    if (blocks.size() != expected_blocks)
        throw std::invalid_argument("block count does not match original_size");

    std::vector<uint8_t> output;
    output.reserve(original_size);
    for (size_t i = 0; i < blocks.size(); ++i) {
        BigUint plaintext = rsa_private_op(blocks[i], key);
        size_t remaining = original_size - output.size();
        size_t expected_bytes = remaining < block_size ? remaining : block_size;
        std::vector<uint8_t> bytes = block_to_bytes(plaintext, expected_bytes);
        output.insert(output.end(), bytes.begin(), bytes.end());
    }
    return output;
}

} // namespace rsa_limb
