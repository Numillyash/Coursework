#pragma once

#include "RSA_Limb.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace rsa_limb {

BigUint biguint_from_hex_be(const std::string& hex);
std::string biguint_to_hex_be(const BigUint& value);

std::string write_public_key_native(const PublicKeyLimb& key);
std::string write_private_key_native(const PrivateKeyLimb& key);

PublicKeyLimb read_public_key_native(const std::string& text);
PrivateKeyLimb read_private_key_native(const std::string& text);

struct NativeBlocks {
    std::string kind;
    size_t original_size;
    size_t block_size;
    std::vector<BigUint> blocks;
};

std::string write_blocks_native(const NativeBlocks& blocks);
NativeBlocks read_blocks_native(const std::string& text);

} // namespace rsa_limb
