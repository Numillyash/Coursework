#include "RSA_Limb_Format.hpp"

#include <limits>
#include <sstream>
#include <stdexcept>

namespace rsa_limb {
namespace {

constexpr const char* KEY_MAGIC = "RSA_LIMB_KEY_V1";
constexpr const char* BLOCK_MAGIC = "RSA_LIMB_BLOCKS_V1";

[[noreturn]] void parse_error(const std::string& message) {
    throw std::invalid_argument(message);
}

void require(bool condition, const std::string& message) {
    if (!condition)
        parse_error(message);
}

std::vector<std::string> split_lines_strict(const std::string& text) {
    std::vector<std::string> lines;
    std::string current;
    for (char ch : text) {
        if (ch == '\r')
            parse_error("CR characters are not supported");
        if (ch == '\n') {
            lines.push_back(current);
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    if (!current.empty())
        lines.push_back(current);
    return lines;
}

int hex_value(char ch) {
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'a' && ch <= 'f')
        return 10 + ch - 'a';
    if (ch >= 'A' && ch <= 'F')
        return 10 + ch - 'A';
    return -1;
}

std::string field_name(const std::string& line) {
    size_t pos = line.find('=');
    require(pos != std::string::npos, "field missing '=': " + line);
    require(pos > 0, "field has empty name");
    return line.substr(0, pos);
}

std::string field_value(const std::string& line) {
    size_t pos = line.find('=');
    require(pos != std::string::npos, "field missing '=': " + line);
    require(pos + 1 < line.size(), "field has empty value: " + line);
    return line.substr(pos + 1);
}

size_t parse_decimal_size(const std::string& text) {
    require(!text.empty(), "empty decimal size");
    size_t result = 0;
    for (char ch : text) {
        require(ch >= '0' && ch <= '9',
                "decimal size contains non-digit");
        const size_t digit = static_cast<size_t>(ch - '0');
        if (result > (std::numeric_limits<size_t>::max() - digit) / 10)
            parse_error("decimal size overflow");
        result = result * 10 + digit;
    }
    return result;
}

std::string key_value_line(const char* name, const BigUint& value) {
    return std::string(name) + "=" + biguint_to_hex_be(value) + "\n";
}

void validate_public_key(const PublicKeyLimb& key) {
    require(!key.n.is_zero(), "public key n must be nonzero");
    require(!key.e.is_zero(), "public key e must be nonzero");
}

void validate_private_key(const PrivateKeyLimb& key) {
    require(!key.n.is_zero(), "private key n must be nonzero");
    require(!key.d.is_zero(), "private key d must be nonzero");
}

struct ParsedKeyFields {
    std::string type;
    bool has_n = false;
    bool has_e = false;
    bool has_d = false;
    BigUint n;
    BigUint e;
    BigUint d;
};

ParsedKeyFields parse_key(const std::string& text) {
    std::vector<std::string> lines = split_lines_strict(text);
    require(!lines.empty(), "key missing magic");
    require(lines[0] == KEY_MAGIC, "key magic mismatch");

    ParsedKeyFields parsed;
    bool has_type = false;
    bool found_end = false;

    for (size_t i = 1; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        if (found_end)
            parse_error("extra data after key terminator");
        if (line == "&") {
            found_end = true;
            continue;
        }

        const std::string name = field_name(line);
        const std::string value = field_value(line);
        if (name == "type") {
            require(!has_type, "duplicated key type");
            has_type = true;
            parsed.type = value;
        } else if (name == "n") {
            require(!parsed.has_n, "duplicated n");
            parsed.has_n = true;
            parsed.n = biguint_from_hex_be(value);
        } else if (name == "e") {
            require(!parsed.has_e, "duplicated e");
            parsed.has_e = true;
            parsed.e = biguint_from_hex_be(value);
        } else if (name == "d") {
            require(!parsed.has_d, "duplicated d");
            parsed.has_d = true;
            parsed.d = biguint_from_hex_be(value);
        } else {
            parse_error("unknown key field: " + name);
        }
    }

    require(found_end, "key missing terminator");
    require(has_type, "key missing type");
    require(parsed.type == "public" || parsed.type == "secret",
            "unknown key type");
    require(parsed.has_n, "key missing n");
    return parsed;
}

} // namespace

BigUint biguint_from_hex_be(const std::string& hex) {
    require(!hex.empty(), "empty hex value");
    BigUint result;
    for (char ch : hex) {
        const int value = hex_value(ch);
        require(value >= 0, "hex value contains invalid character");
        result = result.shift_left_bits(4).add(
                BigUint(static_cast<uint64_t>(value)));
    }
    return result;
}

std::string biguint_to_hex_be(const BigUint& value) {
    if (value.is_zero())
        return "0";

    const size_t nibbles = (value.bit_length() + 3) / 4;
    std::string text;
    text.reserve(nibbles);
    for (size_t index = nibbles; index > 0; --index) {
        const size_t nibble_index = index - 1;
        unsigned nibble = 0;
        for (size_t bit = 0; bit < 4; ++bit) {
            if (value.test_bit(nibble_index * 4 + bit))
                nibble |= 1U << bit;
        }
        text.push_back(static_cast<char>(
                nibble < 10 ? '0' + nibble : 'a' + (nibble - 10)));
    }
    return text;
}

std::string write_public_key_native(const PublicKeyLimb& key) {
    validate_public_key(key);
    std::string text;
    text += KEY_MAGIC;
    text += "\n";
    text += "type=public\n";
    text += key_value_line("n", key.n);
    text += key_value_line("e", key.e);
    text += "&";
    return text;
}

std::string write_private_key_native(const PrivateKeyLimb& key) {
    validate_private_key(key);
    std::string text;
    text += KEY_MAGIC;
    text += "\n";
    text += "type=secret\n";
    text += key_value_line("n", key.n);
    text += key_value_line("d", key.d);
    text += "&";
    return text;
}

PublicKeyLimb read_public_key_native(const std::string& text) {
    ParsedKeyFields parsed = parse_key(text);
    require(parsed.type == "public", "not a public key");
    require(parsed.has_e, "public key missing e");
    require(!parsed.has_d, "public key must not contain d");
    PublicKeyLimb key{parsed.n, parsed.e};
    validate_public_key(key);
    return key;
}

PrivateKeyLimb read_private_key_native(const std::string& text) {
    ParsedKeyFields parsed = parse_key(text);
    require(parsed.type == "secret", "not a secret key");
    require(parsed.has_d, "secret key missing d");
    require(!parsed.has_e, "secret key must not contain e");
    PrivateKeyLimb key{parsed.n, parsed.d};
    validate_private_key(key);
    return key;
}

std::string write_blocks_native(const NativeBlocks& blocks) {
    require(blocks.kind == "cipher" || blocks.kind == "signature",
            "blocks kind must be cipher or signature");
    require(blocks.block_size > 0, "block_size must be nonzero");

    std::ostringstream out;
    out << BLOCK_MAGIC << "\n";
    out << "kind=" << blocks.kind << "\n";
    out << "original_size=" << blocks.original_size << "\n";
    out << "block_size=" << blocks.block_size << "\n";
    out << "block_count=" << blocks.blocks.size() << "\n";
    out << "encoding=hex-be\n";
    for (const BigUint& block : blocks.blocks)
        out << "*c*" << biguint_to_hex_be(block) << "#\n";
    out << "&";
    return out.str();
}

NativeBlocks read_blocks_native(const std::string& text) {
    std::vector<std::string> lines = split_lines_strict(text);
    require(!lines.empty(), "blocks missing magic");
    require(lines[0] == BLOCK_MAGIC, "blocks magic mismatch");

    bool has_kind = false;
    bool has_original_size = false;
    bool has_block_size = false;
    bool has_block_count = false;
    bool has_encoding = false;
    bool found_end = false;
    size_t declared_count = 0;
    NativeBlocks result;

    for (size_t i = 1; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        if (found_end)
            parse_error("extra data after blocks terminator");
        if (line == "&") {
            found_end = true;
            continue;
        }

        if (line.rfind("*c*", 0) == 0) {
            require(has_kind && has_original_size && has_block_size
                            && has_block_count && has_encoding,
                    "block appeared before complete header");
            require(line.size() >= 5 && line.back() == '#',
                    "block missing # terminator");
            const std::string payload = line.substr(3, line.size() - 4);
            require(!payload.empty(), "block has empty hex payload");
            result.blocks.push_back(biguint_from_hex_be(payload));
            continue;
        }

        require(result.blocks.empty(),
                "header field appeared after block data");
        const std::string name = field_name(line);
        const std::string value = field_value(line);
        if (name == "kind") {
            require(!has_kind, "duplicated kind");
            has_kind = true;
            require(value == "cipher" || value == "signature",
                    "unknown block kind");
            result.kind = value;
        } else if (name == "original_size") {
            require(!has_original_size, "duplicated original_size");
            has_original_size = true;
            result.original_size = parse_decimal_size(value);
        } else if (name == "block_size") {
            require(!has_block_size, "duplicated block_size");
            has_block_size = true;
            result.block_size = parse_decimal_size(value);
            require(result.block_size > 0, "block_size must be nonzero");
        } else if (name == "block_count") {
            require(!has_block_count, "duplicated block_count");
            has_block_count = true;
            declared_count = parse_decimal_size(value);
        } else if (name == "encoding") {
            require(!has_encoding, "duplicated encoding");
            has_encoding = true;
            require(value == "hex-be", "unsupported block encoding");
        } else {
            parse_error("unknown block field: " + name);
        }
    }

    require(found_end, "blocks missing terminator");
    require(has_kind, "blocks missing kind");
    require(has_original_size, "blocks missing original_size");
    require(has_block_size, "blocks missing block_size");
    require(has_block_count, "blocks missing block_count");
    require(has_encoding, "blocks missing encoding");
    require(result.blocks.size() == declared_count,
            "block_count does not match number of blocks");
    return result;
}

} // namespace rsa_limb
