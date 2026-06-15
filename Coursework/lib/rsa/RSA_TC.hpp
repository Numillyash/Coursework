#pragma once

#include "BitBigIntTC.hpp"

#include <string>
#include <vector>

namespace rsa_tc {

struct KeyTC {
    bigint::BitBigIntTC n;
    bigint::BitBigIntTC subkey;
};

bigint::BitBigIntTC tc_from_payload(const std::string& payload);
std::string payload_from_tc(const bigint::BitBigIntTC& value);
bigint::BitBigIntTC parse_number_line(
        const std::string& line,
        const std::string& prefix);

KeyTC read_key_tc(const std::string& path, char subkey);
void write_key_tc(
        const std::string& path,
        const bigint::BitBigIntTC& n,
        const bigint::BitBigIntTC& subkey,
        char subkey_name);

std::vector<bigint::BitBigIntTC> read_cipher_blocks_tc(
        const std::string& path);
void write_cipher_blocks_tc(
        const std::string& path,
        const std::vector<bigint::BitBigIntTC>& blocks);

std::vector<bigint::BitBigIntTC> tc_encrypt_bytes(
        const std::string& text,
        const KeyTC& pub);
std::string tc_decrypt_blocks(
        const std::vector<bigint::BitBigIntTC>& blocks,
        const KeyTC& sec);

std::vector<bigint::BitBigIntTC> tc_sign_bytes(
        const std::string& text,
        const KeyTC& sec);
bool tc_check_signature_blocks(
        const std::string& text,
        const std::vector<bigint::BitBigIntTC>& blocks,
        const KeyTC& pub);

} // namespace rsa_tc
