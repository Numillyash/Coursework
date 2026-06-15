#include "RSA_TC.hpp"

#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace rsa_tc {
namespace {

void expect(bool condition, const std::string& message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

bigint::BitBigIntTC tc_from_payload(const std::string& payload)
{
    std::vector<uint8_t> bits;
    for (char ch : payload) {
        int value = ch - 'a';
        expect(value >= 0 && value <= 15,
                "number payload contains non-nibble character");
        for (int i = 0; i < 4; ++i) {
            bits.push_back(static_cast<uint8_t>(value % 2));
            value >>= 1;
        }
    }
    bits.push_back(0);
    return bigint::BitBigIntTC::from_binary_bits(bits);
}

std::string payload_from_tc(const bigint::BitBigIntTC& value)
{
    std::vector<uint8_t> bits = value.raw();
    expect(!bits.empty(), "cannot encode empty BitBigIntTC");

    bits.pop_back();
    while ((bits.size() % 4) != 0) {
        bits.push_back(0);
    }

    std::string payload;
    for (size_t i = 0; i < bits.size(); i += 4) {
        int nibble = 0;
        for (int j = 3; j >= 0; --j) {
            nibble = nibble * 2 + bits[i + static_cast<size_t>(j)];
        }
        payload.push_back(static_cast<char>('a' + nibble));
    }
    return payload;
}

bigint::BitBigIntTC parse_number_line(
        const std::string& line,
        const std::string& prefix)
{
    expect(line.rfind(prefix, 0) == 0, "line has wrong prefix: " + line);
    expect(!line.empty() && line.back() == '#',
            "line does not end with #: " + line);
    return tc_from_payload(line.substr(prefix.size(),
            line.size() - prefix.size() - 1));
}

KeyTC read_key_tc(const std::string& path, char subkey)
{
    std::ifstream in(path);
    expect(static_cast<bool>(in), "failed to open key: " + path);

    std::string n_line;
    std::string subkey_line;
    std::string end_line;
    std::getline(in, n_line);
    std::getline(in, subkey_line);
    std::getline(in, end_line);

    expect(end_line == "&", "key terminator is not &: " + path);

    return KeyTC{parse_number_line(n_line, "_n_"),
            parse_number_line(subkey_line,
                    std::string("_") + subkey + "_")};
}

void write_key_tc(
        const std::string& path,
        const bigint::BitBigIntTC& n,
        const bigint::BitBigIntTC& subkey,
        char subkey_name)
{
    std::ofstream out(path);
    expect(static_cast<bool>(out), "failed to open key for write: " + path);
    out << "_n_" << payload_from_tc(n) << "#\n";
    out << "_" << subkey_name << "_" << payload_from_tc(subkey) << "#\n";
    out << "&";
}

std::vector<bigint::BitBigIntTC> read_cipher_blocks_tc(
        const std::string& path)
{
    std::ifstream in(path);
    expect(static_cast<bool>(in), "failed to open ciphertext: " + path);

    std::vector<bigint::BitBigIntTC> blocks;
    std::string line;
    while (std::getline(in, line)) {
        if (line == "EOF") {
            break;
        }
        blocks.push_back(parse_number_line(line, "_c_"));
    }
    return blocks;
}

void write_cipher_blocks_tc(
        const std::string& path,
        const std::vector<bigint::BitBigIntTC>& blocks)
{
    std::ofstream out(path);
    expect(static_cast<bool>(out), "failed to open ciphertext for write: " + path);
    for (const bigint::BitBigIntTC& block : blocks) {
        out << "_c_" << payload_from_tc(block) << "#\n";
    }
    out << "EOF";
}

std::vector<bigint::BitBigIntTC> tc_encrypt_bytes(
        const std::string& text,
        const KeyTC& pub)
{
    std::vector<bigint::BitBigIntTC> blocks;
    for (unsigned char ch : text) {
        bigint::BitBigIntTC m(static_cast<int64_t>(static_cast<int>(ch) + 100));
        blocks.push_back(m.module_pow_compat_for_testing(pub.subkey, pub.n));
    }
    return blocks;
}

std::string tc_decrypt_blocks(
        const std::vector<bigint::BitBigIntTC>& blocks,
        const KeyTC& sec)
{
    std::string text;
    for (const bigint::BitBigIntTC& block : blocks) {
        bigint::BitBigIntTC m = block.module_pow_compat_for_testing(sec.subkey, sec.n);
        int decoded = m.to_int() - 100;
        expect(decoded >= 0 && decoded <= 255,
                "TC decrypted byte outside unsigned char range");
        text.push_back(static_cast<char>(decoded));
    }
    return text;
}

std::vector<bigint::BitBigIntTC> tc_sign_bytes(
        const std::string& text,
        const KeyTC& sec)
{
    std::vector<bigint::BitBigIntTC> blocks;
    for (unsigned char ch : text) {
        bigint::BitBigIntTC m(static_cast<int64_t>(static_cast<int>(ch) + 100));
        blocks.push_back(m.module_pow_compat_for_testing(sec.subkey, sec.n));
    }
    return blocks;
}

bool tc_check_signature_blocks(
        const std::string& text,
        const std::vector<bigint::BitBigIntTC>& blocks,
        const KeyTC& pub)
{
    size_t index = 0;
    for (const bigint::BitBigIntTC& block : blocks) {
        if (index >= text.size()) {
            return false;
        }
        bigint::BitBigIntTC m = block.module_pow_compat_for_testing(pub.subkey, pub.n);
        int decoded = m.to_int() - 100;
        if (decoded < 0 || decoded > 255
                || static_cast<unsigned char>(decoded)
                        != static_cast<unsigned char>(text[index])) {
            return false;
        }
        ++index;
    }
    return index == text.size();
}

} // namespace rsa_tc
