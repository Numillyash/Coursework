#pragma once

#include "BigUint.hpp"

namespace rsa_limb {

using bigint::BigUint;

struct PublicKeyLimb {
    BigUint n;
    BigUint e;
};

struct PrivateKeyLimb {
    BigUint n;
    BigUint d;
};

struct KeyPairLimb {
    PublicKeyLimb public_key;
    PrivateKeyLimb private_key;
    BigUint p;
    BigUint q;
    BigUint phi;
};

BigUint rsa_public_op(const BigUint& message, const PublicKeyLimb& key);
BigUint rsa_private_op(const BigUint& cipher, const PrivateKeyLimb& key);

KeyPairLimb make_keypair_from_primes(
        const BigUint& p,
        const BigUint& q,
        const BigUint& e);

bool rsa_check_keypair(const KeyPairLimb& keypair);

} // namespace rsa_limb
