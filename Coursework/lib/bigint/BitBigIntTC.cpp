#include "BitBigIntTC.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace bigint
{
// === Constructors ===

BitBigIntTC::BitBigIntTC() : mas_{0, 0}
{
	verify_invariants();
}

BitBigIntTC::BitBigIntTC(uint64_t value)
{
	if (value == 0)
	{
		mas_ = {0, 0};
	}
	else
	{
		// Convert to binary (LSB first)
		while (value > 0)
		{
			mas_.push_back(static_cast<uint8_t>(value & 1));
			value >>= 1;
		}
		// Add sign bit (0 for positive)
		mas_.push_back(0);
	}
	verify_invariants();
}

BitBigIntTC::BitBigIntTC(int64_t value)
{
	uint64_t	uvalue;

	if (value == 0)
	{
		mas_ = {0, 0};
	}
	else if (value > 0)
	{
		uvalue = static_cast<uint64_t>(value);
		while (uvalue > 0)
		{
			mas_.push_back(static_cast<uint8_t>(uvalue & 1));
			uvalue >>= 1;
		}
		// Add sign bit (0 for positive)
		mas_.push_back(0);
	}
	else
	{
		// Negative number: create positive representation, then apply twos complement
		// Safe calculation for INT64_MIN: -(INT64_MIN) is UB, so use -(value+1)+1
		uvalue = static_cast<uint64_t>(-(value + 1)) + 1;
		while (uvalue > 0)
		{
			mas_.push_back(static_cast<uint8_t>(uvalue & 1));
			uvalue >>= 1;
		}
		// Ensure minimum size (at least 2 bits before sign)
		if (mas_.size() < 1)
		{
			mas_.push_back(0);
		}
		// Add sign bit (0 for now, will be set by additional_code)
		mas_.push_back(0);
		// Apply two's complement to make it negative
		additional_code();
	}
	verify_invariants();
}

// === Static factory functions ===

BitBigIntTC BitBigIntTC::from_binary_bits(const std::vector<uint8_t> &raw)
{
	BitBigIntTC	result;

	result.mas_ = raw;
	result.ensure_min_size();
	result.normalize();
	return (result);
}

BitBigIntTC BitBigIntTC::zero()
{
	return (BitBigIntTC());
}

BitBigIntTC BitBigIntTC::one()
{
	BitBigIntTC	result;

	result.mas_[0] = 1;
	result.verify_invariants();
	return (result);
}

// === Representation and inspection ===

std::string BitBigIntTC::to_binary() const
{
	if (mas_.empty())
	{
		return ("(empty - invalid)");
	}

	std::ostringstream oss;

	// All bits except the sign bit (MSB first for readability)
	for (int i = static_cast<int>(mas_.size()) - 2; i >= 0; --i)
	{
		oss << static_cast<int>(mas_[i]);
	}

	// Add sign indicator
	uint8_t sign = sign_bit();
	oss << " (";
	oss << (sign == 0 ? "positive" : "negative");
	oss << ")";

	return (oss.str());
}

bool BitBigIntTC::is_zero() const
{
	if (mas_.size() < 2)
		return (false);
	if (mas_.back() != 0)
		return (false); // Sign bit must be 0

	for (size_t i = 0; i < mas_.size() - 1; ++i)
	{
		if (mas_[i] != 0)
			return (false);
	}
	return (true);
}

int BitBigIntTC::to_int() const
{
	BitBigIntTC	value = *this;
	int			res = 0;
	int			zn = 1;
	int			x = 1;

	if (value.mas_.back())
	{
		zn = -1;
		value.additional_code();
	}
	for (size_t i = 0; i + 1 < value.mas_.size(); ++i)
	{
		res += value.mas_[i] * x;
		x <<= 1;
	}
	return (res * zn);
}

// === Comparison ===

bool BitBigIntTC::is_equal(const BitBigIntTC &other) const
{
	BitBigIntTC lhs = *this;
	BitBigIntTC rhs = other;

	lhs.normalize();
	rhs.normalize();
	return (lhs.mas_ == rhs.mas_);
}

int BitBigIntTC::compare(const BitBigIntTC &other) const
{
	BitBigIntTC lhs = *this;
	BitBigIntTC rhs = other;
	size_t max_size;
	uint8_t lhs_bit;
	uint8_t rhs_bit;

	lhs.normalize();
	rhs.normalize();
	if (lhs.sign_bit() != rhs.sign_bit())
		return (lhs.sign_bit() ? -1 : 1);
	max_size = std::max(lhs.mas_.size(), rhs.mas_.size());
	for (int i = static_cast<int>(max_size) - 2; i >= 0; --i)
	{
		lhs_bit = (i < static_cast<int>(lhs.mas_.size()) - 1)
			? lhs.mas_[static_cast<size_t>(i)] : lhs.sign_bit();
		rhs_bit = (i < static_cast<int>(rhs.mas_.size()) - 1)
			? rhs.mas_[static_cast<size_t>(i)] : rhs.sign_bit();
		if (lhs_bit != rhs_bit)
			return (lhs_bit < rhs_bit ? -1 : 1);
	}
	return (0);
}

bool BitBigIntTC::operator==(const BitBigIntTC &other) const
{
	return (compare(other) == 0);
}

// === Normalization (ported from normalize in bit_LA.c) ===

void BitBigIntTC::normalize()
{
	if (mas_.empty()) {
		mas_.push_back(0);
		mas_.push_back(0);
		return;
	}

	uint8_t sign = mas_.back();
	int end = 0;

	if (sign) {
		for (int i = static_cast<int>(mas_.size()) - 2; i > 0; --i) {
			end = i;
			// ВАЖНО: именно OR, как в legacy
			if (mas_[i] == 0 || mas_[i - 1] == 0) {
				break;
			}
		}
	} else {
		for (int i = static_cast<int>(mas_.size()) - 2; i > 0; --i) {
			if (mas_[i]) {
				end = i;
				break;
			}
		}
	}

	mas_.resize(static_cast<size_t>(end + 2));
}

// === Helper: add_digit (ported from bit_LA.c) ===

void BitBigIntTC::add_digit(uint8_t value)
{
	uint8_t	sign_bit;

	if (mas_.size() == 0)
	{
		mas_.push_back(value);
		mas_.push_back(0); // sign bit
	}
	else
	{
		// Insert value before sign bit, shifting sign bit one position right
		// Save current sign bit position
		sign_bit = mas_.back();
		// Insert new value before the sign bit
		mas_.insert(mas_.end() - 1, value);
		// Restore sign bit at the end
		mas_.back() = sign_bit;
	}
	verify_invariants();
}

// === reverse (ported from bit_LA.c) ===

void BitBigIntTC::reverse()
{
	uint8_t		original_sign;
	int			data_count;
	BitBigIntTC	prom;

	if (mas_.size() <= 1)
	{
		return ;
	}
	// Save original sign bit
	original_sign = mas_.back();
	data_count = static_cast<int>(mas_.size()) - 1;
	// Create temporary with sign-only state (matching legacy init())
	prom.mas_.assign(1, 0); // Only sign bit, like legacy init()
	// Reverse: add data bits in backward order
	for (int idx = data_count - 1; idx >= 0; --idx)
	{
		prom.add_digit(mas_[idx]);
	}
	// Verify: prom must have same size as original after loop
	// (because add_digit adds one element per iteration, starting from size 1)
	assert(prom.mas_.size() == mas_.size()
		&& "Invariant: prom.size() must equal original size after reversing");
	// Copy reversed data back (not changing size of mas_)
	for (int i = 0; i < data_count; ++i)
	{
		mas_[i] = prom.mas_[i];
	}
	// Restore original sign bit
	mas_.back() = original_sign;
	verify_invariants();
}

// === offset_left (ported from bit_LA.c) ===

void BitBigIntTC::offset_left()
{
	reverse();
	add_digit(0);
	// Swap data bits at positions [current_count-2] and [current_count-1]
	if (mas_.size() >= 2)
	{
		std::swap(mas_[mas_.size() - 2], mas_[mas_.size() - 1]);
	}
	reverse();
	normalize();
}

// === offset_right (ported from bit_LA.c) ===

void BitBigIntTC::offset_right()
{
	if (mas_.size() == 2)
	{
		// Number has only data_bit + sign_bit; dividing by 2 gives 0
		mas_ = {0, 0};
	}
	else
	{
		reverse();
		// Move sign bit down one position
		if (mas_.size() > 1)
		{
			mas_[mas_.size() - 2] = mas_[mas_.size() - 1];
			mas_.pop_back();
		}
		reverse();
		normalize();
	}
}

// === Addition (ported from bit_LA.c addition) ===

BitBigIntTC BitBigIntTC::add(const BitBigIntTC &other) const
{
	// Create copies to work with (normalize on entry)
	BitBigIntTC summand = *this;
	BitBigIntTC addend = other;

	summand.normalize();
	addend.normalize();

	// Ensure summand is the larger one (or equal)
	if (summand.mas_.size() < addend.mas_.size())
	{
		std::swap(summand, addend);
	}

	BitBigIntTC carry;
	carry.mas_.assign(1, 0); // init() equivalent
	int oper_sign = 0;
	int real_symb = 0;
	int max_symb = 0;

	// Both negative: convert to two's complement and recurse
	if (summand.mas_.back() && addend.mas_.back())
	{
		// Helper to safely negate a number (handling MIN case)
		auto abs_legacy_safe = [](BitBigIntTC x) -> BitBigIntTC {
			x.normalize();
			if (!x.is_negative())
				return x;

			// Try simple negation
			x.additional_code();
			x.normalize();
			if (!x.is_negative())
				return x;

			// MIN case: extend sign bit and negate again
			x.mas_.push_back(x.mas_.back());
			x.additional_code();
			x.normalize();
			return x;
		};

		BitBigIntTC a = abs_legacy_safe(summand);
		BitBigIntTC b = abs_legacy_safe(addend);
		carry = a.add(b);
		carry.additional_code();

		carry.normalize();
		return carry;
	}

	// Mixed signs or both positive: bit-level addition
	oper_sign = (int)summand.mas_.back() + (int)addend.mas_.back();
	real_symb = static_cast<int>(summand.mas_.size());

	while (!addend.is_zero())
	{
		max_symb = std::max({static_cast<int>(summand.mas_.size()),
				static_cast<int>(addend.mas_.size()),
				static_cast<int>(carry.mas_.size())});

		if (max_symb == 2)
		{
			max_symb++;
			real_symb++;
		}

		// Sign-extend summand
		for (int i = max_symb - (int)summand.mas_.size(); i > 0; i--)
		{
			if (summand.mas_.back())
			{
				summand.add_digit(1);
			}
			else
			{
				summand.add_digit(0);
			}
		}

		// Sign-extend addend
		for (int i = max_symb - (int)addend.mas_.size(); i > 0; i--)
		{
			if (addend.mas_.back())
			{
				addend.add_digit(1);
			}
			else
			{
				addend.add_digit(0);
			}
		}

		// Sign-extend carry
		for (int i = max_symb - (int)carry.mas_.size(); i > 0; i--)
		{
			if (carry.mas_.back())
			{
				carry.add_digit(1);
			}
			else
			{
				carry.add_digit(0);
			}
		}

		// carry = summand & addend (bitwise AND)
		for (int i = 0; i < max_symb; i++)
		{
			carry.mas_[i] = summand.mas_[i] & addend.mas_[i];
		}

		// summand = summand ^ addend (bitwise XOR)
		for (int i = 0; i < max_symb; i++)
		{
			summand.mas_[i] = summand.mas_[i] ^ addend.mas_[i];
		}

		// addend = carry << 1 (left shift of carry)
		addend = carry;
		addend.offset_left();

		// Truncate result if needed for mixed signs
		if (oper_sign == 1)
		{
			if ((int)summand.mas_.size() > real_symb)
			{
				summand.mas_[real_symb] = 0;
			}
		}
	}

	summand.normalize();
	return (summand);
}

// === Subtraction (ported from bit_LA.c difference) ===

BitBigIntTC BitBigIntTC::sub(const BitBigIntTC &other) const
{
	// Port of difference(value1, value2):
	// 1. Copy value2
	// 2. Apply additional_code to the copy
	// 3. Add value1 to the modified copy
	// 4. Return result

	BitBigIntTC b = other;

	// Apply two's complement to b (regardless of sign)
	b.additional_code();

	// Add this to the modified b
	BitBigIntTC result = this->add(b);

	return result;
}

// === Division with remainder (fixed: handles MIN negative, no recursion) ===

DivModTC BitBigIntTC::divmod(const BitBigIntTC &divisor) const
{
	if (divisor.is_zero())
	{
		throw std::runtime_error("Division by zero");
	}

	// --- helpers ---
	auto negate_legacy_inplace = [&](BitBigIntTC &x) {
		x.additional_code();
		x.normalize();
	};

	auto abs_legacy_safe = [&](BitBigIntTC x) -> BitBigIntTC {
		x.normalize();
		if (!x.is_negative())
			return x;

		// Пробуем simple negation
		x.additional_code();
		x.normalize();
		if (!x.is_negative())
			return x;

		// Если все еще negative - это MIN, нужно расширить
		// Expand sign bit and try again
		x.mas_.push_back(x.mas_.back());
		x.additional_code();
		x.normalize();
		return x;
	};

	BitBigIntTC dividend = *this;
	BitBigIntTC dsor = divisor;

	// Convert to absolute values and track signs
	bool a_neg = dividend.is_negative();
	bool b_neg = dsor.is_negative();
	bool q_should_be_negative = (a_neg != b_neg);  // XOR

	if (b_neg)
	{
		dsor = abs_legacy_safe(dsor);
	}

	if (a_neg)
	{
		dividend = abs_legacy_safe(dividend);
	}

	// Теперь оба dividend и dsor - положительные (или нулевые)
	// Выполняем unsigned long division
	BitBigIntTC mod(uint64_t{0});
	BitBigIntTC rem = dividend;
	BitBigIntTC sub = dsor;
	BitBigIntTC add(uint64_t{1});
	BitBigIntTC buff;
	int shifts = 1;

	buff = sub.sub(rem);
	while (buff.is_negative())
	{
		sub.offset_left();
		add.offset_left();
		buff = sub.sub(rem);
		++shifts;
	}

	while (shifts > 0)
	{
		buff = rem.sub(sub);
		while (!buff.is_negative())
		{
			rem = buff;
			mod = mod.add(add);
			buff = rem.sub(sub);
		}
		sub.offset_right();
		add.offset_right();
		--shifts;
	}

	mod.normalize();
	rem.normalize();

	// Euclidean correction для отрицательного dividend
	if (a_neg && !rem.is_zero())
	{
		mod = mod.add(BitBigIntTC(uint64_t{1}));
		rem = dsor.sub(rem);
	}

	// Apply sign corrections
	if (q_should_be_negative && !mod.is_zero())
	{
		// Need to negate quotient. 
		// mod is currently positive. Negate it.
		mod.additional_code();
		mod.normalize();
	}
	else
	{
		mod.normalize();
	}

	if (a_neg && !rem.is_zero())
	{
		// rem уже корректирован выше (rem = dsor - rem)
	}

	rem.normalize();
	return DivModTC{mod, rem};
}

// === Private: Two's complement (additional code) ===

void BitBigIntTC::additional_code()
{
	uint8_t	addit_digit;

	if (!is_zero())
	{
		addit_digit = 1;
		// Invert all bits and add 1
		for (size_t i = 0; i < mas_.size(); i++)
		{
			mas_[i] = !mas_[i];              // NOT
			mas_[i] = mas_[i] ^ addit_digit; // XOR with carry
			if (mas_[i])
			{
				addit_digit = 0;
			}
		}
	}
}

// === Private helpers ===

void BitBigIntTC::verify_invariants() const
{
	// Allow size >= 1 to support legacy init() semantics (temporary sign-only state)
	// Public normalized values will have size >= 2 via ensure_min_size()
	assert(mas_.size() >= 1 && "Invariant: mas_.size() >= 1");

	// Check that all elements are 0 or 1
	for (uint8_t bit : mas_)
	{
		assert((bit == 0 || bit == 1) && "Invariant: each element is 0 or 1");
	}
}

void BitBigIntTC::ensure_min_size()
{
	while (mas_.size() < 2)
	{
		mas_.push_back(0);
	}
}

bool BitBigIntTC::is_negative() const
{
	// Sign bit is the last element; returns 0 if empty (shouldn't happen in normalized state)
	return mas_.empty() ? 0 : mas_.back() == 1;
}

} // namespace bigint
