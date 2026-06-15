#include "RSA_TC.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr int SUCCESS = 0;
constexpr int FAILURE = 1;

void print_help()
{
    std::cout
            << "Usage:\n"
            << "  work1_tc encrypt --infile <in.txt> --pubkey <pub.txt> --outfile <out.txt>\n"
            << "  work1_tc decrypt --infile <in.txt> --secret <sec.txt> --outfile <out.txt>\n"
            << "  work1_tc sign    --infile <in.txt> --secret <sec.txt> --sigfile <sig.txt>\n"
            << "  work1_tc check   --infile <in.txt> --pubkey <pub.txt> --sigfile <sig.txt>\n"
            << "  work1_tc -h|--help\n";
}

std::string read_file_text(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("failed to open input file: " + path);
    }
    return std::string((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
}

void write_file_text(const std::string& path, const std::string& text)
{
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("failed to open output file: " + path);
    }
    out << text;
}

void require_args(
        int argc,
        char* argv[],
        const std::string& mode,
        const std::string& arg1,
        const std::string& arg2,
        const std::string& arg3)
{
    if (argc != 8 || argv[1] != mode || argv[2] != arg1
            || argv[4] != arg2 || argv[6] != arg3) {
        throw std::runtime_error("wrong arguments for " + mode);
    }
}

int run(int argc, char* argv[])
{
    if (argc == 2
            && (std::string(argv[1]) == "-h"
                    || std::string(argv[1]) == "--help")) {
        print_help();
        return SUCCESS;
    }

    if (argc >= 2 && std::string(argv[1]) == "genkey") {
        std::cerr << "genkey is not supported by work1_tc yet" << std::endl;
        return FAILURE;
    }

    if (argc < 2) {
        print_help();
        return FAILURE;
    }

    const std::string mode = argv[1];
    if (mode == "encrypt") {
        require_args(argc, argv, "encrypt", "--infile", "--pubkey", "--outfile");
        const std::string input = read_file_text(argv[3]);
        const rsa_tc::KeyTC pub = rsa_tc::read_key_tc(argv[5], 'e');
        const std::vector<bigint::BitBigIntTC> blocks =
                rsa_tc::tc_encrypt_bytes(input, pub);
        rsa_tc::write_cipher_blocks_tc(argv[7], blocks);
        return SUCCESS;
    }

    if (mode == "decrypt") {
        require_args(argc, argv, "decrypt", "--infile", "--secret", "--outfile");
        const std::vector<bigint::BitBigIntTC> blocks =
                rsa_tc::read_cipher_blocks_tc(argv[3]);
        const rsa_tc::KeyTC sec = rsa_tc::read_key_tc(argv[5], 'd');
        write_file_text(argv[7], rsa_tc::tc_decrypt_blocks(blocks, sec));
        return SUCCESS;
    }

    if (mode == "sign") {
        require_args(argc, argv, "sign", "--infile", "--secret", "--sigfile");
        const std::string input = read_file_text(argv[3]);
        const rsa_tc::KeyTC sec = rsa_tc::read_key_tc(argv[5], 'd');
        const std::vector<bigint::BitBigIntTC> blocks =
                rsa_tc::tc_sign_bytes(input, sec);
        rsa_tc::write_cipher_blocks_tc(argv[7], blocks);
        return SUCCESS;
    }

    if (mode == "check") {
        require_args(argc, argv, "check", "--infile", "--pubkey", "--sigfile");
        const std::string input = read_file_text(argv[3]);
        const rsa_tc::KeyTC pub = rsa_tc::read_key_tc(argv[5], 'e');
        const std::vector<bigint::BitBigIntTC> blocks =
                rsa_tc::read_cipher_blocks_tc(argv[7]);
        if (rsa_tc::tc_check_signature_blocks(input, blocks, pub)) {
            std::cout << "File signature is correct!" << std::endl;
        } else {
            std::cout << "File signature is NOT correct!" << std::endl;
        }
        return SUCCESS;
    }

    throw std::runtime_error("unsupported command: " + mode);
}

} // namespace

int main(int argc, char* argv[])
{
    try {
        return run(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << "work1_tc: " << ex.what() << std::endl;
        return FAILURE;
    }
}
