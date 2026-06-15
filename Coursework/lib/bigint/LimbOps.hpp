#pragma once

#include <cstdint>

namespace bigint {
namespace limb {

using limb_t = uint32_t;
using dlimb_t = uint64_t;

static constexpr unsigned LIMB_BITS = 32;

inline limb_t add_carry(
        limb_t a,
        limb_t b,
        limb_t carry_in,
        limb_t& carry_out) {
    dlimb_t sum = static_cast<dlimb_t>(a)
            + static_cast<dlimb_t>(b)
            + static_cast<dlimb_t>(carry_in);
    carry_out = static_cast<limb_t>(sum >> LIMB_BITS);
    return static_cast<limb_t>(sum);
}

inline limb_t sub_borrow(
        limb_t a,
        limb_t b,
        limb_t borrow_in,
        limb_t& borrow_out) {
    dlimb_t subtrahend = static_cast<dlimb_t>(b)
            + static_cast<dlimb_t>(borrow_in);
    borrow_out = static_cast<limb_t>(
            static_cast<dlimb_t>(a) < subtrahend);
    return static_cast<limb_t>(static_cast<dlimb_t>(a) - subtrahend);
}

inline void mul_wide(limb_t a, limb_t b, limb_t& lo, limb_t& hi) {
    dlimb_t product = static_cast<dlimb_t>(a) * static_cast<dlimb_t>(b);
    lo = static_cast<limb_t>(product);
    hi = static_cast<limb_t>(product >> LIMB_BITS);
}

} // namespace limb
} // namespace bigint
