#include "BitBigIntTC.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using bigint::BitBigIntTC;

namespace {

struct KeyTC {
    BitBigIntTC n;
    BitBigIntTC subkey;
};

[[noreturn]] void fail(const std::string& message)
{
    throw std::runtime_error(message);
}

void expect(bool condition, const std::string& message)
{
    if (!condition) {
        fail(message);
    }
}

std::string shell_quote(const std::string& value)
{
    std::string out = "'";
    for (char c : value) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    out += "'";
    return out;
}

void run_command(const std::string& command)
{
    int rc = std::system(command.c_str());
    if (rc != 0) {
        fail("command failed: " + command);
    }
}

std::string read_file_text(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    expect(static_cast<bool>(in), "failed to open for read: " + path);
    return std::string((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
}

void write_file_text(const std::string& path, const std::string& text)
{
    std::ofstream out(path, std::ios::binary);
    expect(static_cast<bool>(out), "failed to open for write: " + path);
    out << text;
}

BitBigIntTC tc_from_payload(const std::string& payload)
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
    return BitBigIntTC::from_binary_bits(bits);
}

std::string payload_from_tc(const BitBigIntTC& value)
{
    std::vector<uint8_t> bits = value.raw();
    expect(!bits.empty(), "cannot encode empty BitBigIntTC");

    const uint8_t sign = bits.back();
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

    (void)sign;
    return payload;
}

BitBigIntTC parse_number_line(const std::string& line, const std::string& prefix)
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

    KeyTC key{parse_number_line(n_line, "_n_"),
            parse_number_line(subkey_line,
                    std::string("_") + subkey + "_")};
    return key;
}

std::vector<BitBigIntTC> read_cipher_blocks_tc(const std::string& path)
{
    std::ifstream in(path);
    expect(static_cast<bool>(in), "failed to open ciphertext: " + path);

    std::vector<BitBigIntTC> blocks;
    std::string line;
    while (std::getline(in, line)) {
        if (line == "EOF") {
            break;
        }
        blocks.push_back(parse_number_line(line, "_c_"));
    }
    return blocks;
}

void write_cipher_blocks_tc(const std::string& path,
        const std::vector<BitBigIntTC>& blocks)
{
    std::ofstream out(path);
    expect(static_cast<bool>(out), "failed to open ciphertext for write: " + path);
    for (const BitBigIntTC& block : blocks) {
        out << "_c_" << payload_from_tc(block) << "#\n";
    }
    out << "EOF";
}

void write_key_tc(const std::string& path, const BitBigIntTC& n,
        const BitBigIntTC& subkey, char subkey_name)
{
    std::ofstream out(path);
    expect(static_cast<bool>(out), "failed to open key for write: " + path);
    out << "_n_" << payload_from_tc(n) << "#\n";
    out << "_" << subkey_name << "_" << payload_from_tc(subkey) << "#\n";
    out << "&";
}

std::vector<BitBigIntTC> tc_encrypt_bytes(const std::string& text,
        const KeyTC& pub)
{
    std::vector<BitBigIntTC> blocks;
    for (unsigned char ch : text) {
        BitBigIntTC m(static_cast<int64_t>(static_cast<int>(ch) + 100));
        blocks.push_back(m.module_pow_compat_for_testing(pub.subkey, pub.n));
    }
    return blocks;
}

std::string tc_decrypt_blocks(const std::vector<BitBigIntTC>& blocks,
        const KeyTC& sec)
{
    std::string text;
    for (const BitBigIntTC& block : blocks) {
        BitBigIntTC m = block.module_pow_compat_for_testing(sec.subkey, sec.n);
        int decoded = m.to_int() - 100;
        expect(decoded >= 0 && decoded <= 255,
                "TC decrypted byte outside unsigned char range");
        text.push_back(static_cast<char>(decoded));
    }
    return text;
}

std::vector<BitBigIntTC> tc_sign_bytes(const std::string& text,
        const KeyTC& sec)
{
    std::vector<BitBigIntTC> blocks;
    for (unsigned char ch : text) {
        BitBigIntTC m(static_cast<int64_t>(static_cast<int>(ch) + 100));
        blocks.push_back(m.module_pow_compat_for_testing(sec.subkey, sec.n));
    }
    return blocks;
}

bool tc_check_signature_blocks(const std::string& text,
        const std::vector<BitBigIntTC>& blocks,
        const KeyTC& pub)
{
    size_t index = 0;
    for (const BitBigIntTC& block : blocks) {
        if (index >= text.size()) {
            return false;
        }
        BitBigIntTC m = block.module_pow_compat_for_testing(pub.subkey, pub.n);
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

} // namespace

int main()
{
    try {
        const std::string tmp_dir = "test_results/tmp/rsa_tc_smoke";
        std::filesystem::create_directories(tmp_dir);
        std::filesystem::create_directories("test_results/keys");

        const std::string generated_pubkey = "test_results/keys/tc_smoke_pb256.txt";
        const std::string generated_secret = "test_results/keys/tc_smoke_sc256.txt";
        const std::string pubkey = tmp_dir + "/small_pb.txt";
        const std::string secret = tmp_dir + "/small_sc.txt";
        const std::string plain = tmp_dir + "/plain.txt";
        const std::string tc_cipher = tmp_dir + "/tc_cipher.txt";
        const std::string legacy_decrypted = tmp_dir + "/legacy_decrypted.txt";
        const std::string legacy_cipher = tmp_dir + "/legacy_cipher.txt";
        const std::string tc_signature = tmp_dir + "/tc_signature.txt";
        const std::string legacy_signature = tmp_dir + "/legacy_signature.txt";
        const std::string legacy_check_out = tmp_dir + "/legacy_check.out";

        const std::string message = "A";

        run_command("./work1 genkey --size 256 --pubkey "
                + shell_quote(generated_pubkey)
                + " --secret " + shell_quote(generated_secret));
        write_file_text(plain, message);

        (void)read_key_tc(generated_pubkey, 'e');
        (void)read_key_tc(generated_secret, 'd');
        std::cout << "PASS: TC parsed legacy-generated public and secret keys"
                  << std::endl;

        // Small RSA fixture: p=61, q=53, n=3233, phi=3120, e=17, d=2753.
        // It keeps this opt-in smoke target fast while exercising the same
        // legacy file/key formats through work1.
        write_key_tc(pubkey, BitBigIntTC(static_cast<int64_t>(3233)),
                BitBigIntTC(static_cast<int64_t>(17)), 'e');
        write_key_tc(secret, BitBigIntTC(static_cast<int64_t>(3233)),
                BitBigIntTC(static_cast<int64_t>(2753)), 'd');

        KeyTC pub = read_key_tc(pubkey, 'e');
        KeyTC sec = read_key_tc(secret, 'd');
        std::cout << "PASS: TC parsed small legacy-compatible key fixture"
                  << std::endl;

        std::vector<BitBigIntTC> tc_blocks = tc_encrypt_bytes(message, pub);
        write_cipher_blocks_tc(tc_cipher, tc_blocks);
        run_command("./work1 decrypt --infile " + shell_quote(tc_cipher)
                + " --secret " + shell_quote(secret)
                + " --outfile " + shell_quote(legacy_decrypted));
        expect(read_file_text(legacy_decrypted) == message,
                "legacy failed to decrypt TC ciphertext");
        std::cout << "PASS: TC encrypt -> legacy decrypt" << std::endl;

        run_command("./work1 encrypt --infile " + shell_quote(plain)
                + " --pubkey " + shell_quote(pubkey)
                + " --outfile " + shell_quote(legacy_cipher));
        std::vector<BitBigIntTC> legacy_blocks = read_cipher_blocks_tc(legacy_cipher);
        std::string tc_plain = tc_decrypt_blocks(legacy_blocks, sec);
        expect(tc_plain == message, "TC failed to decrypt legacy ciphertext");
        std::cout << "PASS: legacy encrypt -> TC decrypt" << std::endl;

        std::vector<BitBigIntTC> tc_sig_blocks = tc_sign_bytes(message, sec);
        write_cipher_blocks_tc(tc_signature, tc_sig_blocks);
        run_command("./work1 check --infile " + shell_quote(plain)
                + " --pubkey " + shell_quote(pubkey)
                + " --sigfile " + shell_quote(tc_signature)
                + " > " + shell_quote(legacy_check_out) + " 2>&1");
        expect(read_file_text(legacy_check_out)
                        .find("File signature is correct!") != std::string::npos,
                "legacy check rejected TC signature");
        std::cout << "PASS: TC sign -> legacy check" << std::endl;

        run_command("./work1 sign --infile " + shell_quote(plain)
                + " --secret " + shell_quote(secret)
                + " --sigfile " + shell_quote(legacy_signature));
        std::vector<BitBigIntTC> legacy_sig_blocks =
                read_cipher_blocks_tc(legacy_signature);
        expect(tc_check_signature_blocks(message, legacy_sig_blocks, pub),
                "TC check rejected legacy signature");
        std::cout << "PASS: legacy sign -> TC check" << std::endl;

        expect(!tc_check_signature_blocks("B", legacy_sig_blocks, pub),
                "TC check accepted signature for tampered plaintext");
        std::cout << "PASS: TC check rejects tampered plaintext" << std::endl;

        std::cout << "RSA TC smoke test PASSED" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAIL: " << ex.what() << std::endl;
        return 1;
    }
}
