#include "RSA_Limb.hpp"
#include "RSA_Limb_Format.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using bigint::BigUint;
using rsa_limb::NativeBlocks;
using rsa_limb::PrivateKeyLimb;
using rsa_limb::PublicKeyLimb;

namespace {

[[noreturn]] void fail(const std::string& message) {
    std::cerr << "FAIL: " << message << std::endl;
    std::exit(1);
}

void expect(bool condition, const std::string& message) {
    if (!condition)
        fail(message);
}

void expect_eq(const std::string& actual,
        const std::string& expected,
        const std::string& message) {
    if (actual != expected) {
        std::cerr << "FAIL: " << message << std::endl;
        std::cerr << "  actual:   " << actual << std::endl;
        std::cerr << "  expected: " << expected << std::endl;
        std::exit(1);
    }
}

void expect_eq_uint(
        size_t actual,
        size_t expected,
        const std::string& message) {
    if (actual != expected) {
        std::cerr << "FAIL: " << message << std::endl;
        std::cerr << "  actual:   " << actual << std::endl;
        std::cerr << "  expected: " << expected << std::endl;
        std::exit(1);
    }
}

void expect_biguint_eq(
        const BigUint& actual,
        const BigUint& expected,
        const std::string& message) {
    if (actual != expected)
        fail(message);
}

template <typename Fn>
void expect_invalid_argument(Fn fn, const std::string& message) {
    bool threw = false;
    try {
        fn();
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    expect(threw, message);
}

void expect_bytes_eq(
        const std::vector<uint8_t>& actual,
        const std::vector<uint8_t>& expected,
        const std::string& message) {
    if (actual != expected)
        fail(message);
}

std::string block_text(
        const std::string& kind = "cipher",
        const std::string& original_size = "0",
        const std::string& block_size = "1",
        const std::string& block_count = "0",
        const std::string& encoding = "hex-be",
        const std::string& body = "") {
    return std::string("RSA_LIMB_BLOCKS_V1\n")
            + "kind=" + kind + "\n"
            + "original_size=" + original_size + "\n"
            + "block_size=" + block_size + "\n"
            + "block_count=" + block_count + "\n"
            + "encoding=" + encoding + "\n"
            + body
            + "&";
}

void test_hex_roundtrip() {
    std::cout << "\n=== hex BigUint roundtrip ===" << std::endl;

    const std::vector<std::pair<BigUint, std::string>> cases = {
        {BigUint(0), "0"},
        {BigUint(1), "1"},
        {BigUint(0x1234), "1234"},
        {BigUint(UINT32_MAX), "ffffffff"},
        {BigUint(uint64_t{1} << 32), "100000000"},
        {BigUint(UINT64_MAX), "ffffffffffffffff"},
        {BigUint(1).shift_left_bits(128), "100000000000000000000000000000000"},
        {BigUint(1).shift_left_bits(256), "10000000000000000000000000000000000000000000000000000000000000000"},
    };

    for (const auto& test : cases) {
        expect_eq(rsa_limb::biguint_to_hex_be(test.first), test.second,
                "BigUint to hex");
        expect_biguint_eq(rsa_limb::biguint_from_hex_be(test.second),
                test.first, "hex to BigUint");
    }

    expect_biguint_eq(rsa_limb::biguint_from_hex_be("00000123"),
            BigUint(0x123), "leading zero hex parses canonically");
    expect_eq(rsa_limb::biguint_to_hex_be(
                      rsa_limb::biguint_from_hex_be("ABCDEF")),
            "abcdef", "uppercase input normalizes to lowercase output");
}

void test_hex_invalid_cases() {
    std::cout << "\n=== hex invalid cases ===" << std::endl;

    expect_invalid_argument([] {
        (void)rsa_limb::biguint_from_hex_be("");
    }, "empty hex throws");
    expect_invalid_argument([] {
        (void)rsa_limb::biguint_from_hex_be("12xz");
    }, "invalid hex throws");
    expect_invalid_argument([] {
        (void)rsa_limb::biguint_from_hex_be("12 34");
    }, "whitespace inside hex throws");
    expect_invalid_argument([] {
        (void)rsa_limb::biguint_from_hex_be("0x1234");
    }, "0x prefix is rejected");
}

void test_key_roundtrips() {
    std::cout << "\n=== key write/read roundtrip ===" << std::endl;

    PublicKeyLimb pub{BigUint(3233), BigUint(17)};
    std::string pub_text = rsa_limb::write_public_key_native(pub);
    expect(pub_text.find("RSA_LIMB_KEY_V1") != std::string::npos,
            "public key contains magic");
    PublicKeyLimb read_pub = rsa_limb::read_public_key_native(pub_text);
    expect_biguint_eq(read_pub.n, pub.n, "public n roundtrip");
    expect_biguint_eq(read_pub.e, pub.e, "public e roundtrip");

    PrivateKeyLimb sec{BigUint(3233), BigUint(2753)};
    std::string sec_text = rsa_limb::write_private_key_native(sec);
    PrivateKeyLimb read_sec = rsa_limb::read_private_key_native(sec_text);
    expect_biguint_eq(read_sec.n, sec.n, "secret n roundtrip");
    expect_biguint_eq(read_sec.d, sec.d, "secret d roundtrip");
}

void test_key_negative_cases() {
    std::cout << "\n=== key parser negative cases ===" << std::endl;

    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("type=public\nn=1\ne=1\n&");
    }, "missing key magic throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("BAD\n"
                                               "type=public\nn=1\ne=1\n&");
    }, "wrong key magic throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=unknown\nn=1\ne=1\n&");
    }, "unknown key type throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\nn=1\nn=2\ne=1\n&");
    }, "duplicated n throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\nn=1\ne=1\ne=2\n&");
    }, "duplicated e throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_private_key_native("RSA_LIMB_KEY_V1\n"
                                                "type=secret\nn=1\nd=1\nd=2\n&");
    }, "duplicated d throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\ne=1\n&");
    }, "missing n throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\nn=1\n&");
    }, "missing e throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_private_key_native("RSA_LIMB_KEY_V1\n"
                                                "type=secret\nn=1\n&");
    }, "missing d throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\nn=0\ne=1\n&");
    }, "zero n throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\nn=1\ne=0\n&");
    }, "zero e throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_private_key_native("RSA_LIMB_KEY_V1\n"
                                                "type=secret\nn=1\nd=0\n&");
    }, "zero d throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\nn=1\ne=1\nd=1\n&");
    }, "public key containing d throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_private_key_native("RSA_LIMB_KEY_V1\n"
                                                "type=secret\nn=1\nd=1\ne=1\n&");
    }, "secret key containing e throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\nn=zz\ne=1\n&");
    }, "invalid key hex throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\nn=1\ne=1\n");
    }, "missing key terminator throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_public_key_native("RSA_LIMB_KEY_V1\n"
                                               "type=public\nn=1\ne=1\n&\nextra");
    }, "extra data after key terminator throws");
}

void test_blocks_roundtrip() {
    std::cout << "\n=== blocks write/read roundtrip ===" << std::endl;

    const std::vector<NativeBlocks> cases = {
        NativeBlocks{"cipher", 0, 1, {}},
        NativeBlocks{"signature", 0, 1, {}},
        NativeBlocks{"cipher", 1, 1, {BigUint(0)}},
        NativeBlocks{"signature", 5, 2,
                {BigUint(1), BigUint(0x1234), BigUint(1).shift_left_bits(128)}},
    };

    for (const NativeBlocks& test : cases) {
        NativeBlocks parsed =
                rsa_limb::read_blocks_native(rsa_limb::write_blocks_native(test));
        expect_eq(parsed.kind, test.kind, "blocks kind roundtrip");
        expect_eq_uint(parsed.original_size, test.original_size,
                "original_size roundtrip");
        expect_eq_uint(parsed.block_size, test.block_size,
                "block_size roundtrip");
        expect_eq_uint(parsed.blocks.size(), test.blocks.size(),
                "block count roundtrip");
        for (size_t i = 0; i < test.blocks.size(); ++i)
            expect_biguint_eq(parsed.blocks[i], test.blocks[i],
                    "block value roundtrip");
    }

    std::string zero_text = rsa_limb::write_blocks_native(
            NativeBlocks{"cipher", 1, 1, {BigUint(0)}});
    expect(zero_text.find("*c*0#") != std::string::npos,
            "zero block is encoded as 0");
}

void test_blocks_negative_cases() {
    std::cout << "\n=== blocks parser negative cases ===" << std::endl;

    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native("kind=cipher\n"
                                           "original_size=0\nblock_size=1\n"
                                           "block_count=0\nencoding=hex-be\n&");
    }, "missing block magic throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native("BAD\n"
                                           "kind=cipher\noriginal_size=0\n"
                                           "block_size=1\nblock_count=0\n"
                                           "encoding=hex-be\n&");
    }, "wrong block magic throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text("bad"));
    }, "wrong kind throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native("RSA_LIMB_BLOCKS_V1\n"
                                           "kind=cipher\nblock_size=1\n"
                                           "block_count=0\nencoding=hex-be\n&");
    }, "missing original_size throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native("RSA_LIMB_BLOCKS_V1\n"
                                           "kind=cipher\noriginal_size=0\n"
                                           "block_count=0\nencoding=hex-be\n&");
    }, "missing block_size throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text(
                "cipher", "0", "0", "0", "hex-be"));
    }, "zero block_size throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native("RSA_LIMB_BLOCKS_V1\n"
                                           "kind=cipher\noriginal_size=0\n"
                                           "block_size=1\nencoding=hex-be\n&");
    }, "missing block_count throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text(
                "cipher", "12x", "1", "0", "hex-be"));
    }, "invalid decimal size throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text(
                "cipher", "-1", "1", "0", "hex-be"));
    }, "negative decimal text throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text(
                "cipher", "0", "1", "0", "base64"));
    }, "unsupported encoding throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text(
                "cipher", "1", "1", "2", "hex-be", "*c*1#\n"));
    }, "block_count mismatch throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text(
                "cipher", "1", "1", "1", "hex-be", "_c_1#\n"));
    }, "invalid block prefix throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text(
                "cipher", "1", "1", "1", "hex-be", "*c*1\n"));
    }, "missing block # throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text(
                "cipher", "1", "1", "1", "hex-be", "*c*z#\n"));
    }, "invalid block hex throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native("RSA_LIMB_BLOCKS_V1\n"
                                           "kind=cipher\noriginal_size=0\n"
                                           "block_size=1\nblock_count=0\n"
                                           "encoding=hex-be\n");
    }, "missing block terminator throws");
    expect_invalid_argument([] {
        (void)rsa_limb::read_blocks_native(block_text()
                + "\nextra");
    }, "extra data after block terminator throws");
}

void test_integration_smoke() {
    std::cout << "\n=== RSA_Limb native format integration smoke ==="
              << std::endl;

    rsa_limb::KeyPairLimb keypair = rsa_limb::make_keypair_from_primes(
            BigUint(257), BigUint(263), BigUint(17));
    PublicKeyLimb pub = rsa_limb::read_public_key_native(
            rsa_limb::write_public_key_native(keypair.public_key));
    PrivateKeyLimb sec = rsa_limb::read_private_key_native(
            rsa_limb::write_private_key_native(keypair.private_key));

    const std::vector<uint8_t> plaintext = {0, 1, 2, 3, 0, 255};
    std::vector<BigUint> cipher = rsa_limb::rsa_encrypt_bytes(plaintext, pub);
    NativeBlocks cipher_blocks{
        "cipher",
        plaintext.size(),
        rsa_limb::max_plaintext_block_bytes(pub),
        cipher,
    };
    NativeBlocks parsed_cipher = rsa_limb::read_blocks_native(
            rsa_limb::write_blocks_native(cipher_blocks));
    std::vector<uint8_t> decrypted = rsa_limb::rsa_decrypt_blocks(
            parsed_cipher.blocks, sec, parsed_cipher.original_size);
    expect_bytes_eq(decrypted, plaintext,
            "native cipher blocks decrypt to plaintext");

    std::vector<BigUint> signature = rsa_limb::rsa_sign_bytes(plaintext, sec);
    NativeBlocks signature_blocks{
        "signature",
        plaintext.size(),
        rsa_limb::max_plaintext_block_bytes(sec),
        signature,
    };
    NativeBlocks parsed_signature = rsa_limb::read_blocks_native(
            rsa_limb::write_blocks_native(signature_blocks));
    expect(rsa_limb::rsa_check_signature_bytes(
                   plaintext, parsed_signature.blocks, pub),
            "native signature blocks check");
}

} // namespace

int main() {
    std::cout << "RSA_Limb Native Format Tests\n";
    std::cout << "============================\n" << std::endl;

    test_hex_roundtrip();
    test_hex_invalid_cases();
    test_key_roundtrips();
    test_key_negative_cases();
    test_blocks_roundtrip();
    test_blocks_negative_cases();
    test_integration_smoke();

    std::cout << "\nRSA_Limb native format tests PASSED" << std::endl;
    return 0;
}
