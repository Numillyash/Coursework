#include "RSA_TC.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <ctime>
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

struct PrimeSource {
    const char* path;
    int bit_size;
};

PrimeSource prime_source_for_key_size(int key_size)
{
    switch (key_size) {
    case 256:
        return PrimeSource{"primes_128_bit.blackleague", 128};
    case 512:
        return PrimeSource{"primes_256_bit.blackleague", 256};
    case 1024:
        return PrimeSource{"primes_512_bit.blackleague", 512};
    case 2048:
        return PrimeSource{"primes_1024_bit.blackleague", 1024};
    default:
        throw std::runtime_error("unsupported key size: "
                + std::to_string(key_size));
    }
}

int parse_prime_line_number(const std::string& line, size_t& payload_start)
{
    expect(line.size() >= 5 && line[0] == '_',
            "prime line has wrong prefix: " + line);

    size_t pos = 1;
    int line_number = 0;
    expect(pos < line.size() && line[pos] >= '0' && line[pos] <= '9',
            "prime line has empty numeric prefix: " + line);
    while (pos < line.size() && line[pos] != '_') {
        expect(line[pos] >= '0' && line[pos] <= '9',
                "prime line contains non-decimal prefix: " + line);
        line_number = line_number * 10 + (line[pos] - '0');
        ++pos;
    }

    expect(pos < line.size() && line[pos] == '_',
            "prime line prefix is not terminated: " + line);
    payload_start = pos + 1;
    return line_number;
}

GeneratedKeyTC generate_key_from_primes(
        const bigint::BitBigIntTC& p,
        const bigint::BitBigIntTC& q)
{
    const bigint::BitBigIntTC one(static_cast<int64_t>(1));
    const bigint::BitBigIntTC zero(static_cast<int64_t>(0));
    const bigint::BitBigIntTC e(static_cast<int64_t>(65537));

    bigint::BitBigIntTC n = p.multiplication_compat_for_testing(q);
    bigint::BitBigIntTC p_minus_one = p.sub(one);
    bigint::BitBigIntTC q_minus_one = q.sub(one);
    bigint::BitBigIntTC phi =
            p_minus_one.multiplication_compat_for_testing(q_minus_one);

    std::array<bigint::BitBigIntTC, 4> values{
            one,
            zero,
            zero,
            one,
    };
    (void)phi.euclide_algorithm_modifyed_compat_for_testing(e, values);
    bigint::BitBigIntTC d = values[1];

    if (d.sign_bit() != 0) {
        d = d.add(phi);
    }

    return GeneratedKeyTC{KeyTC{n, e}, KeyTC{n, d}, phi};
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

bigint::BitBigIntTC read_prime_tc(
        const std::string& path,
        int line_number,
        int bit_size)
{
    expect(line_number >= 1 && line_number <= 500,
            "prime line number must be in 1..500");

    std::ifstream in(path);
    expect(static_cast<bool>(in), "failed to open prime file: " + path);

    std::string line;
    while (std::getline(in, line)) {
        if (line == "&") {
            break;
        }

        size_t payload_start = 0;
        const int current_line = parse_prime_line_number(line, payload_start);
        expect(!line.empty() && line.back() == '#',
                "prime line does not end with #: " + line);

        if (current_line != line_number) {
            continue;
        }

        const size_t payload_end = line.size() - 1;
        expect(payload_end >= payload_start,
                "prime line has empty payload: " + line);
        expect(static_cast<int>(payload_end - payload_start) == bit_size,
                "prime bit size does not match expected size");

        std::vector<uint8_t> bits;
        bits.reserve(static_cast<size_t>(bit_size) + 1);
        for (size_t i = payload_start; i < payload_end; ++i) {
            expect(line[i] == '0' || line[i] == '1',
                    "prime payload contains non-bit character");
            bits.push_back(static_cast<uint8_t>(line[i] - '0'));
        }
        bits.push_back(0);
        return bigint::BitBigIntTC::from_binary_bits(bits);
    }

    throw std::runtime_error("prime line not found");
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

GeneratedKeyTC generate_key_tc_for_testing(
        int key_size,
        int line_p,
        int line_q)
{
    expect(line_p != line_q, "prime line numbers must be distinct");
    const PrimeSource source = prime_source_for_key_size(key_size);
    bigint::BitBigIntTC p =
            read_prime_tc(source.path, line_p, source.bit_size);
    bigint::BitBigIntTC q =
            read_prime_tc(source.path, line_q, source.bit_size);
    return generate_key_from_primes(p, q);
}

GeneratedKeyTC generate_key_tc(int key_size)
{
    int line_p = -1;
    int line_q = -1;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    while (line_p == line_q) {
        line_p = std::rand() % 500 + 1;
        line_q = std::rand() % 500 + 1;
    }
    return generate_key_tc_for_testing(key_size, line_p, line_q);
}

} // namespace rsa_tc
