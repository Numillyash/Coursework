#include "RSA_Limb.hpp"

#include <stdexcept>

namespace rsa_limb {

namespace {

void validate_public_key(const PublicKeyLimb& key) {
    if (key.n.is_zero())
        throw std::invalid_argument("rsa_public_op zero modulus");
    if (key.e.is_zero())
        throw std::invalid_argument("rsa_public_op zero exponent");
}

void validate_private_key(const PrivateKeyLimb& key) {
    if (key.n.is_zero())
        throw std::invalid_argument("rsa_private_op zero modulus");
    if (key.d.is_zero())
        throw std::invalid_argument("rsa_private_op zero exponent");
}

} // namespace

BigUint rsa_public_op(const BigUint& message, const PublicKeyLimb& key) {
    validate_public_key(key);
    if (message.compare(key.n) >= 0)
        throw std::invalid_argument("rsa_public_op message out of range");
    return BigUint::mod_pow(message, key.e, key.n);
}

BigUint rsa_private_op(const BigUint& cipher, const PrivateKeyLimb& key) {
    validate_private_key(key);
    if (cipher.compare(key.n) >= 0)
        throw std::invalid_argument("rsa_private_op cipher out of range");
    return BigUint::mod_pow(cipher, key.d, key.n);
}

KeyPairLimb make_keypair_from_primes(
        const BigUint& p,
        const BigUint& q,
        const BigUint& e) {
    if (p.compare(BigUint::one()) <= 0)
        throw std::invalid_argument("make_keypair_from_primes p <= 1");
    if (q.compare(BigUint::one()) <= 0)
        throw std::invalid_argument("make_keypair_from_primes q <= 1");
    if (p == q)
        throw std::invalid_argument("make_keypair_from_primes p == q");
    if (e.is_zero())
        throw std::invalid_argument("make_keypair_from_primes zero e");

    BigUint n = p.mul_schoolbook(q);
    BigUint phi = p.sub_abs(BigUint::one()).mul_schoolbook(
            q.sub_abs(BigUint::one()));
    if (BigUint::gcd(e, phi) != BigUint::one())
        throw std::invalid_argument("make_keypair_from_primes e not coprime");

    BigUint d = BigUint::mod_inverse(e, phi);
    return KeyPairLimb{
        PublicKeyLimb{n, e},
        PrivateKeyLimb{n, d},
        p,
        q,
        phi,
    };
}

bool rsa_check_keypair(const KeyPairLimb& keypair) {
    if (keypair.public_key.n != keypair.private_key.n)
        return false;
    if (keypair.public_key.n != keypair.p.mul_schoolbook(keypair.q))
        return false;
    BigUint check = BigUint::mod_mul(
            keypair.public_key.e,
            keypair.private_key.d,
            keypair.phi);
    return check == BigUint::one();
}

} // namespace rsa_limb
