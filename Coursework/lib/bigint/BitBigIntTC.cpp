#include "BitBigIntTC.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace bigint
{
namespace
{
constexpr int FT_DIRECT = -1;
constexpr int FT_INVERSE = 1;

constexpr float Rcoef[14] =
	{-1.0000000000000000F, 0.0000000000000000F, 0.7071067811865475F,
	 0.9238795325112867F, 0.9807852804032304F, 0.9951847266721969F,
	 0.9987954562051724F, 0.9996988186962042F, 0.9999247018391445F,
	 0.9999811752826011F, 0.9999952938095761F, 0.9999988234517018F,
	 0.9999997058628822F, 0.9999999264657178F};
constexpr float Icoef[14] =
	{0.0000000000000000F, -1.0000000000000000F, -0.7071067811865474F,
	 -0.3826834323650897F, -0.1950903220161282F, -0.0980171403295606F,
	 -0.0490676743274180F, -0.0245412285229122F, -0.0122715382857199F,
	 -0.0061358846491544F, -0.0030679567629659F, -0.0015339801862847F,
	 -0.0007669903187427F, -0.0003834951875714F};

bool number_is_2_pow_k(int x)
{
	return ((!((x) & ((x) - 1))) && ((x) > 1));
}

bool fft_compat(float *Rdat, float *Idat, int N, int LogN, int Ft_Flag)
{
	if ((Rdat == nullptr) || (Idat == nullptr))
		return false;
	if ((N > 16384) || (N < 1))
		return false;
	if (!number_is_2_pow_k(N))
		return false;
	if ((LogN < 2) || (LogN > 14))
		return false;
	if ((Ft_Flag != FT_DIRECT) && (Ft_Flag != FT_INVERSE))
		return false;

	int i, j, n, k, io, ie, in, nn;
	float ru, iu, rtp, itp, rtq, itq, rw, iw, sr;

	nn = N >> 1;
	ie = N;
	for (n = 1; n <= LogN; n++)
	{
		rw = Rcoef[LogN - n];
		iw = Icoef[LogN - n];
		if (Ft_Flag == FT_INVERSE)
			iw = -iw;
		in = ie >> 1;
		ru = 1.0F;
		iu = 0.0F;
		for (j = 0; j < in; j++)
		{
			for (i = j; i < N; i += ie)
			{
				io = i + in;
				rtp = Rdat[i] + Rdat[io];
				itp = Idat[i] + Idat[io];
				rtq = Rdat[i] - Rdat[io];
				itq = Idat[i] - Idat[io];
				Rdat[io] = rtq * ru - itq * iu;
				Idat[io] = itq * ru + rtq * iu;
				Rdat[i] = rtp;
				Idat[i] = itp;
			}

			sr = ru;
			ru = ru * rw - iu * iw;
			iu = iu * rw + sr * iw;
		}

		ie >>= 1;
	}

	for (j = i = 1; i < N; i++)
	{
		if (i < j)
		{
			io = i - 1;
			in = j - 1;
			rtp = Rdat[in];
			itp = Idat[in];
			Rdat[in] = Rdat[io];
			Idat[in] = Idat[io];
			Rdat[io] = rtp;
			Idat[io] = itp;
		}

		k = nn;
		while (k < j)
		{
			j = j - k;
			k >>= 1;
		}
		j = j + k;
	}

	if (Ft_Flag == FT_DIRECT)
		return true;

	rw = 1.0F / N;
	for (i = 0; i < N; i++)
	{
		Rdat[i] *= rw;
		Idat[i] *= rw;
	}
	return true;
}

std::vector<uint8_t> convert_number_compat(const BitBigIntTC &value,
		int &count, int &ln_count)
{
	int num = 1;
	ln_count = 0;
	std::vector<uint8_t> input = value.raw();

	count = value.current_count();
	do {
		num <<= 1;
		ln_count += 1;
	} while (count > num);
	input.resize(static_cast<size_t>(num), 0);
	count = num;
	return input;
}
}

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

// === easy_mult compatibility helper (ported from bit_LA.c easy_mult) ===

BitBigIntTC BitBigIntTC::easy_mult_compat_for_testing(
		const BitBigIntTC &other) const
{
	int	a = this->to_int();
	int	b = other.to_int();
	int	product = a * b;

	return (BitBigIntTC(static_cast<int64_t>(product)));
}

// === multiply_furie compatibility helper (ported from bit_LA.c multiply_furie) ===

BitBigIntTC BitBigIntTC::multiply_furie_compat_for_testing(
		const BitBigIntTC &other) const
{
	if (this->is_zero() || other.is_zero())
		return (BitBigIntTC(static_cast<int64_t>(0)));

	int i;
	BitBigIntTC value_1 = *this;
	BitBigIntTC value_2 = other;

	if ((value_1.current_count() >> 1) >= value_2.current_count())
	{
		BitBigIntTC tmp1 = value_1;
		BitBigIntTC tmp2 = value_1;
		tmp2.mas_.resize(static_cast<size_t>(value_2.current_count()));
		tmp2.mas_[tmp2.mas_.size() - 1] = 0;
		for (i = 0; i < value_2.current_count() - 1; i++)
		{
			tmp1.offset_right();
		}
		BitBigIntTC result1 = tmp1.multiply_furie_compat_for_testing(value_2);
		BitBigIntTC result2 = tmp2.multiply_furie_compat_for_testing(value_2);
		for (i = 0; i < value_2.current_count() - 1; i++)
		{
			result1.offset_left();
		}
		BitBigIntTC result = result1.add(result2);
		result.normalize();
		return (result);
	}
	// Preserve legacy suspicious/dead condition exactly.
	if ((value_2.current_count() >> 1) >= value_2.current_count())
	{
		BitBigIntTC tmp1 = value_2;
		BitBigIntTC tmp2 = value_2;
		tmp2.mas_.resize(static_cast<size_t>(value_1.current_count()));
		tmp2.mas_[tmp2.mas_.size() - 1] = 0;
		for (i = 0; i < value_1.current_count() - 1; i++)
		{
			tmp1.offset_right();
		}
		BitBigIntTC result1 = tmp1.multiply_furie_compat_for_testing(value_1);
		BitBigIntTC result2 = tmp2.multiply_furie_compat_for_testing(value_1);
		for (i = 0; i < value_1.current_count() - 1; i++)
		{
			result1.offset_left();
		}
		BitBigIntTC result = result1.add(result2);
		result.normalize();
		return (result);
	}

	int len, ln_count, len_1, ln_count_1, len_2, ln_count_2;
	std::vector<uint8_t> mas_1 = convert_number_compat(value_1, len_1, ln_count_1);
	std::vector<uint8_t> mas_2 = convert_number_compat(value_2, len_2, ln_count_2);

	if (len_1 > len_2)
	{
		mas_1.resize(static_cast<size_t>(len_1 * 2), 0);
		mas_2.resize(static_cast<size_t>(len_1 * 2), 0);
		len = len_1 << 1;
		ln_count = ++ln_count_1;
	}
	else
	{
		mas_1.resize(static_cast<size_t>(len_2 * 2), 0);
		mas_2.resize(static_cast<size_t>(len_2 * 2), 0);
		len = len_2 << 1;
		ln_count = ++ln_count_2;
	}

	std::vector<float> Re_1(static_cast<size_t>(len));
	std::vector<float> Im_1(static_cast<size_t>(len));
	std::vector<float> Re_2(static_cast<size_t>(len));
	std::vector<float> Im_2(static_cast<size_t>(len));
	std::vector<float> Re_3(static_cast<size_t>(len));
	std::vector<float> Im_3(static_cast<size_t>(len));

	for (i = 0; i < len; i++)
	{
		Re_1[static_cast<size_t>(i)] = static_cast<float>(mas_1[static_cast<size_t>(i)]);
		Im_1[static_cast<size_t>(i)] = 0.0F;
	}
	for (i = 0; i < len; i++)
	{
		Re_2[static_cast<size_t>(i)] = static_cast<float>(mas_2[static_cast<size_t>(i)]);
		Im_2[static_cast<size_t>(i)] = 0.0F;
	}
	for (i = 0; i < len; i++)
	{
		Re_3[static_cast<size_t>(i)] = 0.0F;
		Im_3[static_cast<size_t>(i)] = 0.0F;
	}

	(void)fft_compat(Re_1.data(), Im_1.data(), len, ln_count, FT_DIRECT);
	(void)fft_compat(Re_2.data(), Im_2.data(), len, ln_count, FT_DIRECT);
	for (i = 0; i < len; i++)
	{
		Re_3[static_cast<size_t>(i)] = Re_1[static_cast<size_t>(i)]
			* Re_2[static_cast<size_t>(i)] - Im_1[static_cast<size_t>(i)]
			* Im_2[static_cast<size_t>(i)];
		Im_3[static_cast<size_t>(i)] = Im_1[static_cast<size_t>(i)]
			* Re_2[static_cast<size_t>(i)] + Re_1[static_cast<size_t>(i)]
			* Im_2[static_cast<size_t>(i)];
	}
	(void)fft_compat(Re_3.data(), Im_3.data(), len, ln_count, FT_INVERSE);

	std::vector<uint8_t> result_bits(static_cast<size_t>(len + 1), 0);
	for (i = 0; i < len; i++)
	{
		int rounded = static_cast<int>(Re_3[static_cast<size_t>(i)] < 0
			? (Re_3[static_cast<size_t>(i)] - 0.5F)
			: (Re_3[static_cast<size_t>(i)] + 0.5F));
		result_bits[static_cast<size_t>(i)] =
			static_cast<uint8_t>(result_bits[static_cast<size_t>(i)] + rounded);
		if (result_bits[static_cast<size_t>(i)] >> 1)
		{
			result_bits[static_cast<size_t>(i + 1)] =
				static_cast<uint8_t>(result_bits[static_cast<size_t>(i)] >> 1);
			result_bits[static_cast<size_t>(i)] &= 1;
		}
	}

	for (i = len - 1; i > 0 && result_bits[static_cast<size_t>(i - 1)] == 0; i--)
		;

	BitBigIntTC result;
	result.mas_.assign(result_bits.begin(), result_bits.begin() + i);
	result.mas_.push_back(0);
	result.normalize();
	return (result);
}

// === karatsuba compatibility helper (ported from bit_LA.c karatsuba) ===

BitBigIntTC BitBigIntTC::karatsuba_compat_for_testing(
		const BitBigIntTC &other) const
{
	BitBigIntTC value1 = *this;
	BitBigIntTC value2 = other;

	if (value1.current_count() < 5 && value2.current_count() < 5)
	{
		if (value1.is_zero() || value2.is_zero())
			return (BitBigIntTC(static_cast<int64_t>(0)));
		return (value1.easy_mult_compat_for_testing(value2));
	}

	BitBigIntTC res;
	int n;
	int k;
	int iter;

	if (value1.current_count() < 256 && value2.current_count() < 256)
	{
		if (value1.is_zero() || value2.is_zero())
			return (BitBigIntTC(static_cast<int64_t>(0)));
		res = value1.multiply_furie_compat_for_testing(value2);
		return (res);
	}

	n = std::max(value1.current_count(), value2.current_count()) - 1;
	k = n / 2;

	if (n <= 5)
	{
		return (value1.easy_mult_compat_for_testing(value2));
	}
	else
	{
		BitBigIntTC v1 = value1;
		BitBigIntTC v2 = value2;
		BitBigIntTC a;
		BitBigIntTC b;
		BitBigIntTC c;
		BitBigIntTC d;

		a.mas_.assign(1, 0);
		b.mas_.assign(1, 0);
		c.mas_.assign(1, 0);
		d.mas_.assign(1, 0);

		for (iter = n - v1.current_count(); iter > 0; iter--)
		{
			v1.add_digit(0);
		}
		for (iter = n - v2.current_count(); iter > 0; iter--)
		{
			v2.add_digit(0);
		}

		for (iter = 0; iter < k; iter++)
		{
			b.add_digit(v1.mas_[static_cast<size_t>(iter)]);
			d.add_digit(v2.mas_[static_cast<size_t>(iter)]);
		}

		for (iter = k; iter < n; iter++)
		{
			a.add_digit(v1.mas_[static_cast<size_t>(iter)]);
			c.add_digit(v2.mas_[static_cast<size_t>(iter)]);
		}

		BitBigIntTC p1 = b.karatsuba_compat_for_testing(d);
		BitBigIntTC p2 = a.karatsuba_compat_for_testing(c);
		BitBigIntTC buff1 = a.add(b);
		BitBigIntTC buff2 = c.add(d);
		BitBigIntTC t = buff1.karatsuba_compat_for_testing(buff2);

		buff1 = t;
		t = buff1.sub(p1);

		buff1 = t;
		t = buff1.sub(p2);

		for (iter = 0; iter < 2 * k; iter++)
		{
			p2.offset_left();
		}
		for (iter = 0; iter < k; iter++)
		{
			t.offset_left();
		}

		buff1 = res;
		res = buff1.add(p2);

		buff1 = res;
		res = buff1.add(p1);

		buff1 = res;
		res = buff1.add(t);
		return (res);
	}
}

// === public multiplication compatibility helper (ported from bit_LA.c multiplication) ===

BitBigIntTC BitBigIntTC::multiplication_compat_for_testing(
		const BitBigIntTC &other) const
{
	if (is_zero() || other.is_zero())
		return (BitBigIntTC(static_cast<int64_t>(0)));

	BitBigIntTC a = *this;
	BitBigIntTC b = other;
	int sign = (a.mas_[a.mas_.size() - 1] != 0)
		+ (b.mas_[b.mas_.size() - 1] != 0);

	if (a.mas_[a.mas_.size() - 1])
	{
		a.additional_code();
	}
	if (b.mas_[b.mas_.size() - 1])
	{
		b.additional_code();
	}

	BitBigIntTC result = a.karatsuba_compat_for_testing(b);

	if (sign == 1)
	{
		result.additional_code();
	}
	result.normalize();
	return (result);
}

// === module_pow compatibility helper (ported from bit_LA.c module_pow) ===

BitBigIntTC BitBigIntTC::module_pow_compat_for_testing(
		const BitBigIntTC &exponent,
		const BitBigIntTC &modulus) const
{
	BitBigIntTC d = divmod(modulus).r;
	BitBigIntTC ost = d;

	if (d.is_zero())
	{
		return (ost);
	}

	BitBigIntTC iterator = exponent;
	ost = BitBigIntTC(static_cast<int64_t>(1));
	BitBigIntTC buff2 = *this;

	while (!iterator.is_zero())
	{
		if (iterator.mas_[0] == 1)
		{
			BitBigIntTC buff = ost.multiplication_compat_for_testing(buff2);
			ost = buff.divmod(modulus).r;
		}

		BitBigIntTC buff = buff2.multiplication_compat_for_testing(buff2);
		buff2 = buff.divmod(modulus).r;

		iterator.offset_right();
	}

	BitBigIntTC buff = ost;
	ost = buff.divmod(modulus).r;
	ost.normalize();
	return (ost);
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
