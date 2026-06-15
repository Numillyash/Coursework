#pragma once

#include "BitBigIntTC.hpp"

#include <string>
#include <vector>

namespace rsa_tc {

struct KeyTC {
    bigint::BitBigIntTC n;
    bigint::BitBigIntTC subkey;
};

struct GeneratedKeyTC {
    KeyTC public_key;
    KeyTC secret_key;
    bigint::BitBigIntTC phi;
};

bigint::BitBigIntTC tc_from_payload(const std::string& payload);
std::string payload_from_tc(const bigint::BitBigIntTC& value);
bigint::BitBigIntTC parse_number_line(
        const std::string& line,
        const std::string& prefix);

bigint::BitBigIntTC read_prime_tc(
        const std::string& path,
        int line_number,
        int bit_size);

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

GeneratedKeyTC generate_key_tc_for_testing(
        int key_size,
        int line_p,
        int line_q);
GeneratedKeyTC generate_key_tc(int key_size);

} // namespace rsa_tc
