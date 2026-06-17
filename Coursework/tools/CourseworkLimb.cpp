#include "RSA_Limb.hpp"
#include "RSA_Limb_Format.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr int SUCCESS = 0;
constexpr int FAILURE = 1;

void print_help() {
    std::cout
            << "Usage:\n"
            << "  work1_limb encrypt --infile <input.bin> --pubkey <public.key> --outfile <cipher.txt>\n"
            << "  work1_limb decrypt --infile <cipher.txt> --secret <secret.key> --outfile <output.bin>\n"
            << "  work1_limb -h|--help\n";
}

std::vector<uint8_t> read_file_bytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        throw std::runtime_error("failed to open input file: " + path);
    return std::vector<uint8_t>(
            std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>());
}

std::string read_file_text(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        throw std::runtime_error("failed to open input file: " + path);
    return std::string(
            std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>());
}

void write_file_bytes(
        const std::string& path,
        const std::vector<uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    if (!out)
        throw std::runtime_error("failed to open output file: " + path);
    for (uint8_t byte : bytes)
        out.put(static_cast<char>(byte));
}

void write_file_text(const std::string& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    if (!out)
        throw std::runtime_error("failed to open output file: " + path);
    out << text;
}

void require_args(
        int argc,
        char* argv[],
        const std::string& mode,
        const std::string& arg1,
        const std::string& arg2,
        const std::string& arg3) {
    if (argc != 8 || argv[1] != mode || argv[2] != arg1
            || argv[4] != arg2 || argv[6] != arg3) {
        throw std::runtime_error("wrong arguments for " + mode);
    }
}

int run(int argc, char* argv[]) {
    if (argc == 2
            && (std::string(argv[1]) == "-h"
                    || std::string(argv[1]) == "--help")) {
        print_help();
        return SUCCESS;
    }

    if (argc < 2) {
        print_help();
        return FAILURE;
    }

    const std::string mode = argv[1];
    if (mode == "encrypt") {
        require_args(argc, argv, "encrypt", "--infile", "--pubkey", "--outfile");
        const std::vector<uint8_t> input = read_file_bytes(argv[3]);
        const rsa_limb::PublicKeyLimb pub =
                rsa_limb::read_public_key_native(read_file_text(argv[5]));
        const size_t block_size = rsa_limb::max_plaintext_block_bytes(pub);
        const std::vector<bigint::BigUint> blocks =
                rsa_limb::rsa_encrypt_bytes(input, pub);
        rsa_limb::NativeBlocks native_blocks{
            "cipher",
            input.size(),
            block_size,
            blocks,
        };
        write_file_text(argv[7],
                rsa_limb::write_blocks_native(native_blocks));
        return SUCCESS;
    }

    if (mode == "decrypt") {
        require_args(argc, argv, "decrypt", "--infile", "--secret", "--outfile");
        const rsa_limb::NativeBlocks blocks =
                rsa_limb::read_blocks_native(read_file_text(argv[3]));
        if (blocks.kind != "cipher")
            throw std::runtime_error("input blocks are not cipher blocks");
        const rsa_limb::PrivateKeyLimb sec =
                rsa_limb::read_private_key_native(read_file_text(argv[5]));
        const size_t expected_block_size =
                rsa_limb::max_plaintext_block_bytes(sec);
        if (blocks.block_size != expected_block_size)
            throw std::runtime_error("cipher block_size does not match key");
        const std::vector<uint8_t> output = rsa_limb::rsa_decrypt_blocks(
                blocks.blocks, sec, blocks.original_size);
        write_file_bytes(argv[7], output);
        return SUCCESS;
    }

    throw std::runtime_error("unsupported command: " + mode);
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        return run(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << "work1_limb: " << ex.what() << std::endl;
        return FAILURE;
    }
}
