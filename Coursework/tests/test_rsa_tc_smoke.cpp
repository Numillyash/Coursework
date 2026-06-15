#include "RSA_TC.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using bigint::BitBigIntTC;
using rsa_tc::KeyTC;

namespace {

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

        (void)rsa_tc::read_key_tc(generated_pubkey, 'e');
        (void)rsa_tc::read_key_tc(generated_secret, 'd');
        std::cout << "PASS: TC parsed legacy-generated public and secret keys"
                  << std::endl;

        // Small RSA fixture: p=61, q=53, n=3233, phi=3120, e=17, d=2753.
        // It keeps this opt-in smoke target fast while exercising the same
        // legacy file/key formats through work1.
        rsa_tc::write_key_tc(pubkey, BitBigIntTC(static_cast<int64_t>(3233)),
                BitBigIntTC(static_cast<int64_t>(17)), 'e');
        rsa_tc::write_key_tc(secret, BitBigIntTC(static_cast<int64_t>(3233)),
                BitBigIntTC(static_cast<int64_t>(2753)), 'd');

        KeyTC pub = rsa_tc::read_key_tc(pubkey, 'e');
        KeyTC sec = rsa_tc::read_key_tc(secret, 'd');
        std::cout << "PASS: TC parsed small legacy-compatible key fixture"
                  << std::endl;

        std::vector<BitBigIntTC> tc_blocks = rsa_tc::tc_encrypt_bytes(message, pub);
        rsa_tc::write_cipher_blocks_tc(tc_cipher, tc_blocks);
        run_command("./work1 decrypt --infile " + shell_quote(tc_cipher)
                + " --secret " + shell_quote(secret)
                + " --outfile " + shell_quote(legacy_decrypted));
        expect(read_file_text(legacy_decrypted) == message,
                "legacy failed to decrypt TC ciphertext");
        std::cout << "PASS: TC encrypt -> legacy decrypt" << std::endl;

        run_command("./work1 encrypt --infile " + shell_quote(plain)
                + " --pubkey " + shell_quote(pubkey)
                + " --outfile " + shell_quote(legacy_cipher));
        std::vector<BitBigIntTC> legacy_blocks =
                rsa_tc::read_cipher_blocks_tc(legacy_cipher);
        std::string tc_plain = rsa_tc::tc_decrypt_blocks(legacy_blocks, sec);
        expect(tc_plain == message, "TC failed to decrypt legacy ciphertext");
        std::cout << "PASS: legacy encrypt -> TC decrypt" << std::endl;

        std::vector<BitBigIntTC> tc_sig_blocks =
                rsa_tc::tc_sign_bytes(message, sec);
        rsa_tc::write_cipher_blocks_tc(tc_signature, tc_sig_blocks);
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
                rsa_tc::read_cipher_blocks_tc(legacy_signature);
        expect(rsa_tc::tc_check_signature_blocks(message, legacy_sig_blocks, pub),
                "TC check rejected legacy signature");
        std::cout << "PASS: legacy sign -> TC check" << std::endl;

        expect(!rsa_tc::tc_check_signature_blocks("B", legacy_sig_blocks, pub),
                "TC check accepted signature for tampered plaintext");
        std::cout << "PASS: TC check rejects tampered plaintext" << std::endl;

        std::cout << "RSA TC smoke test PASSED" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAIL: " << ex.what() << std::endl;
        return 1;
    }
}
