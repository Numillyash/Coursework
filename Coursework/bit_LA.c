//
// Created by Georgul on 02.11.2021.
//
#include "bit_LA.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

#define XOR(a, b) a ^ b

#define NOT(a) !a

#define AND(a, b) a &b

#pragma region counters
#define ull unsigned long long
ull multy_fur = 0;
ull additional_code_ = 0;
ull init_ = 0;
ull copy_ = 0;
ull normalize_ = 0;
ull add_digit_ = 0;
ull offset_right_ = 0;
ull offset_left_ = 0;
ull reverse_ = 0;
ull is_zero_ = 0;
ull is_equal_ = 0;
ull addition_ = 0;
ull difference_ = 0;
ull easy_mult_ = 0;
ull karatsuba_ = 0;
ull multiplication_ = 0;
ull division_with_module_ = 0;
ull module_pow_ = 0;
#pragma endregion counters

void _log_counters()
{
	_log("add_digit_");
	_log_ull(add_digit_);
	_log("init_");
	_log_ull(init_);
	_log("reverse_");
	_log_ull(reverse_);
	_log("normalize_");
	_log_ull(normalize_);
	_log("copy_");
	_log_ull(copy_);
	_log("is_zero_");
	_log_ull(is_zero_);
	_log("offset_left_");
	_log_ull(offset_left_);
	_log("addition_");
	_log_ull(addition_);
	_log("additional_code_");
	_log_ull(additional_code_);
	_log("difference_");
	_log_ull(difference_);
	_log("offset_right_");
	_log_ull(offset_right_);
	_log("division_with_module_");
	_log_ull(division_with_module_);
	_log("multy_fur");
	_log_ull(multy_fur);
	_log("karatsuba_");
	_log_ull(karatsuba_);
	_log("multiplication_");
	_log_ull(multiplication_);
	_log("module_pow_");
	_log_ull(module_pow_);
	_log("is_equal_");
	_log_ull(is_equal_);
	_log("easy_mult_");
	_log_ull(easy_mult_);
}

#pragma region furie

#define NUMBER_IS_2_POW_K(x) ((!((x) & ((x)-1))) && ((x) > 1)) // x is pow(2, k), k=1,2, ...
#define FT_DIRECT -1										   // Direct transform.
#define FT_INVERSE 1										   // Inverse transform.
#define bool char
#define true 1
#define false 0
#define max_error 0.001

static const float Rcoef[14] =
	{-1.0000000000000000F, 0.0000000000000000F, 0.7071067811865475F,
	 0.9238795325112867F, 0.9807852804032304F, 0.9951847266721969F,
	 0.9987954562051724F, 0.9996988186962042F, 0.9999247018391445F,
	 0.9999811752826011F, 0.9999952938095761F, 0.9999988234517018F,
	 0.9999997058628822F, 0.9999999264657178F};
static const float Icoef[14] =
	{0.0000000000000000F, -1.0000000000000000F, -0.7071067811865474F,
	 -0.3826834323650897F, -0.1950903220161282F, -0.0980171403295606F,
	 -0.0490676743274180F, -0.0245412285229122F, -0.0122715382857199F,
	 -0.0061358846491544F, -0.0030679567629659F, -0.0015339801862847F,
	 -0.0007669903187427F, -0.0003834951875714F};

BOOL FFT(float *Rdat, float *Idat, int N, int LogN, int Ft_Flag)
{
	// parameters error check:
	if ((Rdat == NULL) || (Idat == NULL))
		return FALSE;
	if ((N > 16384) || (N < 1))
		return FALSE;
	if (!NUMBER_IS_2_POW_K(N))
		return FALSE;
	if ((LogN < 2) || (LogN > 14))
		return FALSE;
	if ((Ft_Flag != FT_DIRECT) && (Ft_Flag != FT_INVERSE))
		return FALSE;

	register int i, j, n, k, io, ie, in, nn;
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
		return TRUE;

	rw = 1.0F / N;

	for (i = 0; i < N; i++)
	{
		Rdat[i] *= rw;
		Idat[i] *= rw;
	}

	return TRUE;
}

int convert_number(number *value, char **input, int *count, int *ln_count)
{
	int num = 1, i;
	*ln_count = 0;
	*input = (char *)malloc(value->current_count);
	if (*input == NULL)
	{
		return MEMORY_ALLOCATION_FAILURE;
	}
	memcpy(*input, value->mas, value->current_count);
	*count = value->current_count;
	do
	{
		num <<= 1;
		*ln_count += 1;
	} while (*count > num);
	*input = (char *)realloc(*input, num);
	if (*input == NULL)
	{
		return MEMORY_ALLOCATION_FAILURE;
	}
	for (; *count < num; (*count)++)
		(*input)[(*count)] = 0;
}

number multiply_furie(number *value_1, number *value_2)
{
	multy_fur++;
	if (is_zero(value_1) || is_zero(value_2))
		return int_to_number(0);
	int i; // iterator

	{
		if ((value_1->current_count >> 1) >= value_2->current_count)
		{
			number tmp1 = copy(value_1);
			number tmp2 = copy(value_1);
			tmp2.current_count = value_2->current_count;
			tmp2.mas[tmp2.current_count - 1] = 0;
			for (i = 0; i < value_2->current_count - 1; i++)
			{
				offset_right(&tmp1);
			}
			number result1 = multiply_furie(&tmp1, value_2);
			number result2 = multiply_furie(&tmp2, value_2);
			for (i = 0; i < value_2->current_count - 1; i++)
			{
				offset_left(&result1);
			}
			number result = addition(&result1, &result2);
			clear_mem(&tmp1);
			clear_mem(&tmp2);
			clear_mem(&result1);
			clear_mem(&result2);
			normalize(&result);
			return result;
		}
		if ((value_2->current_count >> 1) >= value_2->current_count)
		{
			number tmp1 = copy(value_2);
			number tmp2 = copy(value_2);
			tmp2.current_count = value_1->current_count;
			tmp2.mas[tmp2.current_count - 1] = 0;
			for (i = 0; i < value_1->current_count - 1; i++)
			{
				offset_right(&tmp1);
			}
			number result1 = multiply_furie(&tmp1, value_1);
			number result2 = multiply_furie(&tmp2, value_1);
			for (i = 0; i < value_1->current_count - 1; i++)
			{
				offset_left(&result1);
			}
			number result = addition(&result1, &result2);
			clear_mem(&tmp1);
			clear_mem(&tmp2);
			clear_mem(&result1);
			clear_mem(&result2);
			normalize(&result);
			return result;
		}
	}

	int len, ln_count, len_1, ln_count_1, len_2, ln_count_2;
	char *mas_1, *mas_2;

	// printf("Start furie\nv1:%d, v2:%d\n", value_1->current_count, value_2->current_count);
	//  Convert number to char arrays
	convert_number(value_1, &mas_1, &len_1, &ln_count_1);
	convert_number(value_2, &mas_2, &len_2, &ln_count_2);
	// Alignment
	if (len_1 > len_2)
	{
		mas_1 = (char *)realloc(mas_1, len_1 * 2);
		for (i = len_1; i < len_1 * 2; i++)
		{
			mas_1[i] = 0;
		}
		mas_2 = (char *)realloc(mas_2, len_1 * 2);
		for (i = len_2; i < len_1 * 2; i++)
		{
			mas_2[i] = 0;
		}
		len = len_1 << 1;
		ln_count = ++ln_count_1;
	}
	else
	{
		mas_1 = (char *)realloc(mas_1, len_2 * 2);
		for (i = len_1; i < len_2 * 2; i++)
		{
			mas_1[i] = 0;
		}
		mas_2 = (char *)realloc(mas_2, len_2 * 2);
		for (i = len_2; i < len_2 * 2; i++)
		{
			mas_2[i] = 0;
		}
		len = len_2 << 1;
		ln_count = ++ln_count_2;
	}

	// Init furie arrays
	float *Re_1 = (float *)malloc(len * sizeof(float));
	float *Im_1 = (float *)malloc(len * sizeof(float));
	float *Re_2 = (float *)malloc(len * sizeof(float));
	float *Im_2 = (float *)malloc(len * sizeof(float));
	// answer
	float *Re_3 = (float *)malloc(len * sizeof(float));
	float *Im_3 = (float *)malloc(len * sizeof(float));
	// fill furie arrays from string
	for (i = 0; i < len; i++)
	{
		Re_1[i] = (float)mas_1[i];
		Im_1[i] = 0.0;
	}
	for (i = 0; i < len; i++)
	{
		Re_2[i] = (float)mas_2[i];
		Im_2[i] = 0.0;
	}
	free(mas_1);
	free(mas_2);
	// answer
	for (i = 0; i < len; i++)
	{
		Re_3[i] = 0.0;
		Im_3[i] = 0.0;
	}
	// Straight FFT
	FFT(Re_1, Im_1, len, ln_count, FT_DIRECT);
	FFT(Re_2, Im_2, len, ln_count, FT_DIRECT);
	// Multy
	for (i = 0; i < len; i++)
	{
		Re_3[i] = Re_1[i] * Re_2[i] - Im_1[i] * Im_2[i];
		Im_3[i] = Im_1[i] * Re_2[i] + Re_1[i] * Im_2[i];
	}
	free(Re_1);
	free(Im_1);
	free(Re_2);
	free(Im_2);
	// Rev FFT
	FFT(Re_3, Im_3, len, ln_count, FT_INVERSE);
	// Convert to number
	number result = init();
	result.size = len;
	result.current_count = len;
	free(result.mas);
	free(Im_3);
	result.mas = (char *)malloc(len);
	memset(result.mas, 0, len);
	for (i = 0; i < len; i++)
	{
		result.mas[i] += (int)(Re_3[i] < 0 ? (Re_3[i] - 0.5) : (Re_3[i] + 0.5));
		if (result.mas[i] >> 1)
		{
			result.mas[i + 1] = result.mas[i] >> 1;
			result.mas[i] &= 1;
		}
	}
	free(Re_3);
	for (i = len - 1; i > 0 && result.mas[i - 1] == 0; i--)
		;
	result.mas = (char *)realloc(result.mas, i + 1);
	result.current_count = i;
	add_digit(&result, 0);
	swap(result.mas[result.current_count - 1], result.mas[result.current_count - 2]);
	// printf("End furie\n CC = %d, size = %d\n", result.current_count, result.size);
	normalize(&result);
	// printf("End normalize\n CC = %d, size = %d\n", result.current_count, result.size);
	return result;
}

#pragma endregion furie

void additional_code(number *value)
{
	additional_code_++;
	if (!is_zero(value))
	{
		uint8_t addit_digit = 1;
		int iter;

		for (iter = 0; iter < value->current_count; iter++)
		{
			value->mas[iter] = NOT(value->mas[iter]);
			value->mas[iter] = XOR(value->mas[iter], addit_digit);
			if (value->mas[iter])
				addit_digit = 0;
		}
	}
}

number init()
{
	init_++;
	number result = {1, 1};
	result.mas = (uint8_t *)malloc(sizeof(uint8_t));
	if (result.mas == NULL)
	{
		_log("Memory allocation failure in init() function");
		exit(MEMORY_ALLOCATION_FAILURE);
	}
	result.mas[0] = 0;
	return result;
}

number copy(number *value)
{
	copy_++;
	// TODO: Избавиться от add_digit и использовать фор для прямого добавления

	number result = init();
	clear_mem(&result);
	result.mas = (uint8_t *)malloc(sizeof(uint8_t) * (value->size));
	if (result.mas == NULL)
	{
		_log("Memory allocation failure in add_digit() function (buffer)");
		exit(MEMORY_ALLOCATION_FAILURE);
	}
	memcpy(result.mas, value->mas, value->size);
	result.current_count = value->current_count;
	result.size = value->size;
	return result;
}

number int_to_number(int value)
{
	int ostatok;
	number result = init();

	// TODO: Двоичная система: % заменяется на &

	if (value < 0)
	{
		value *= -1;
		while (value > 0)
		{
			ostatok = value % NUMBER_SYSTEM_BASE;
			add_digit(&result, ostatok);
			value >>= 1;
		}
		if (result.current_count == 0)
			add_digit(&result, 0);
		additional_code(&result);
	}
	else
	{
		while (value > 0)
		{
			ostatok = value % NUMBER_SYSTEM_BASE;
			add_digit(&result, ostatok);
			value >>= 1;
		}
		if (result.current_count == 1)
		{
			add_digit(&result, 0);
		}
	}

	return result;
}

int number_to_int(number *value)
{
	int res = 0, i, zn = 1;
	int x = 1; // степень двойки

	if (value->mas[value->current_count - 1])
	{
		zn = -1;
		additional_code(value); // nonadditional_code is used
		for (i = 0; i < value->current_count - 1; i++)
		{
			res += value->mas[i] * x;
			x <<= 1;
		}
	}
	else
	{
		for (i = 0; i < value->current_count - 1; i++)
		{
			res += value->mas[i] * x;
			x <<= 1;
		}
	}
	res *= zn;
	return res;
}

void normalize(number *value)
{
	normalize_++;
	// normalize_n(value);
	// return;
	int i; // iterator
	int end = value->current_count - 2;
	if (value->mas[value->current_count - 1])
	{
		for (i = value->current_count - 2; i > 0; i--)
		{
			end = i;
			if (value->mas[i] == 0 || value->mas[i - 1] == 0)
			{
				break;
			}
		}
	}
	else
	{
		for (i = value->current_count - 2; i >= 0; i--)
		{
			end = i;
			if (value->mas[i] != 0)
			{
				break;
			}
		}
	}

	char *buff = (char *)malloc(end + 2);
	if (buff == NULL)
	{
		_log("Memory allocation failure in init() function");
		exit(MEMORY_ALLOCATION_FAILURE);
	}
	memcpy(buff, value->mas, end + 1);
	buff[end + 1] = value->mas[value->current_count - 1];
	free(value->mas);
	value->mas = buff;
	if (value->mas == NULL)
	{
		_log("Memory allocation failure in init() function");
		exit(MEMORY_ALLOCATION_FAILURE);
	}
	value->size = end + 2;
	value->current_count = end + 2;
}

void clear_mem(number *value)
{
	free(value->mas);
	value->mas = NULL;
}

void add_digit(number *object, uint8_t value)
{
	add_digit_++;
	// TODO malloc в realloc?
	int iter;
	uint8_t *buff;

	if (object->current_count < object->size)
	{
		object->mas[object->current_count] = object->mas[object->current_count - 1];
		object->mas[object->current_count - 1] = value;
		object->current_count += 1;
	}
	else
	{
		buff = (uint8_t *)malloc(sizeof(uint8_t) * (object->current_count));
		if (buff == NULL)
		{
			_log("Memory allocation failure in add_digit() function (buffer)");
			exit(MEMORY_ALLOCATION_FAILURE);
		}

		memcpy(buff, object->mas, object->current_count);

		clear_mem(object);
		object->mas = (uint8_t *)malloc(sizeof(uint8_t) * (object->size) * 2);
		if (object->mas == NULL)
		{
			_log("Memory allocation failure in add_digit() function (object)");
			exit(MEMORY_ALLOCATION_FAILURE);
		}

		memcpy(object->mas, buff, object->current_count);

		free(buff);
		object->mas[object->current_count] = object->mas[object->current_count - 1];
		object->mas[object->current_count - 1] = value;
		object->current_count += 1;
		object->size *= 2;
	}
}

void offset_right(number *object)
{
	offset_right_++;
	// TODO: Не нравится куча реверсов
	if (object->current_count == 2)
	{
		clear_mem(object);
		*object = int_to_number(0);
	}
	else
	{
		// prom = init
		// копируем символы с первого (не с 0)
		// clear_mem(value)
		// value = copy(&prom)
		//
		// v[i] = v[i+1]
		// object->current_count -= 1;

		reverse(object);
		object->mas[object->current_count - 2] = object->mas[object->current_count - 1];
		object->current_count -= 1;
		reverse(object);

		normalize(object);
	}
}

void offset_left(number *value)
{
	offset_left_++;
	// _log("offset_left");
	// debug_log(value);
	// TODO: Не нравится куча реверсов
	//
	reverse(value);
	//

	add_digit(value, 0);
	//

	swap(value->mas[value->current_count - 2], value->mas[value->current_count - 1]);
	//

	reverse(value);
	//
	// debug_log(value);
	//_log("normalize");
	normalize(value);
	// debug_log(value);
	//_log("end");
}

void reverse(number *value)
{
	reverse_++;
	// TODO: Обоийтись без лишнего инита и копировать по другому?

	number prom = init();
	int i; // iterator
	for (i = value->current_count - 2; i >= 0; i--)
	{
		add_digit(&prom, value->mas[i]);
	}
	for (i = 0; i <= prom.current_count - 2; i++)
	{
		value->mas[i] = prom.mas[i];
	}
	clear_mem(&prom);
}

void print_number_as_is(number *value)
{
	int i; // iterator
	for (i = 0; i <= value->current_count - 2; i++)
	{
		printf("%d", (int)value->mas[i]);
		if (i % 4 == 3)
			printf(" ");
	}
	printf(" %d", (int)value->mas[value->current_count - 1]);
	printf("\n");
}

void print_number_decimal(number *value)
{
	int y;	   // iterator
	int x = 1; // stepen 2
	int s = 0; // summa
	if (value->mas[value->current_count - 1])
	{
		printf("-");
		additional_code(value); // nonadditional_code is used

		for (y = 0; y < value->current_count - 1; y++)
		{
			s += value->mas[y] * x;
			x <<= 1;
		}
		printf("%d\n", s);
		additional_code(value);
	}
	else
	{
		for (y = 0; y < value->current_count - 1; y++)
		{
			s += value->mas[y] * x;
			x <<= 1;
		}
		printf("%d\n", s);
	}
}

void print_number(number *_value)
{
	int i; // iterator
	number value = copy(_value);
	printf("%d ", (int)value.mas[value.current_count - 1]);
	while (((value.current_count) - 1) % 4 != 0)
	{
		add_digit(&value, value.mas[value.current_count - 1] ? 1 : 0);
	}
	for (i = value.current_count - 2; i >= 0; i--)
	{
		printf("%d", (int)value.mas[i]);
		if (i % 4 == 0)
			printf(" ");
	}
	normalize(&value);

	printf("\n");
}

void debug_log(number *value)
{
	int i, n = value->current_count;
	char buff[4000];
	buff[0] = value->mas[n - 1] + '0';
	buff[1] = ' ';

	//    |
	// 100101 cc=6
	// 1 01001
	//  |

	for (i = n - 2; i >= 0; i--)
	{
		buff[n - i] = value->mas[i] + '0';
	}
	buff[n + 1] = '\0';
	// printf("%s\n", buff);
	_log(buff);
}

BOOL is_zero(number *object)
{
	is_zero_++;
	int iterator = 0;

	for (iterator = 0; iterator < object->current_count; iterator++)
	{
		if (object->mas[iterator] != 0)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL is_equal(number *value1, number *value2)
{
	is_equal_++;
	short iterator = 0;
	normalize(value1);
	normalize(value2);

	if (value1->current_count != value2->current_count)
		return FALSE;
	for (iterator = 0; iterator < value1->current_count; iterator++)
	{
		if (value1->mas[iterator] != value2->mas[iterator])
			return FALSE;
	}

	return TRUE;
}

number addition(number *value1, number *value2)
{
	addition_++;
	number summand, addend;
	number carry = init();
	int real_symb = 0;
	int oper_sign = 0;
	int iter = 0;
	int max_symb = 0;

	normalize(value1);
	normalize(value2);

	if ((value1->current_count) >= (value2->current_count))
	{
		summand = copy(value1);
		addend = copy(value2);
	}
	else
	{
		summand = copy(value2);
		addend = copy(value1);
	}

	if (summand.mas[summand.current_count - 1] && addend.mas[addend.current_count - 1])
	{
		clear_mem(&carry);
		additional_code(&summand); // nonadditional_code is used
		additional_code(&addend);  // nonadditional_code is used
		carry = addition(&summand, &addend);
		additional_code(&carry);
		clear_mem(&addend);
		clear_mem(&summand);

		normalize(&carry);
		return carry;
	}

	oper_sign = summand.mas[summand.current_count - 1] + addend.mas[addend.current_count - 1];
	real_symb = max(summand.current_count, addend.current_count);

	while (!is_zero(&addend))
	{
		max_symb = max(summand.current_count, addend.current_count);
		max_symb = max(max_symb, carry.current_count);
		if (max_symb == 2)
		{
			max_symb++;
			real_symb++;
		}

		for (iter = max_symb - summand.current_count; iter > 0; iter--)
		{
			if (summand.mas[summand.current_count - 1])
				add_digit(&summand, 1);
			else
				add_digit(&summand, 0);
		}
		for (iter = max_symb - addend.current_count; iter > 0; iter--)
		{
			if (addend.mas[addend.current_count - 1])
				add_digit(&addend, 1);
			else
				add_digit(&addend, 0);
		}
		for (iter = max_symb - carry.current_count; iter > 0; iter--)
		{
			if (carry.mas[carry.current_count - 1])
				add_digit(&carry, 1);
			else
				add_digit(&carry, 0);
		}

		// carry = (summand & addend);
		for (iter = 0; iter < max_symb; iter++)
		{
			carry.mas[iter] = AND(summand.mas[iter], addend.mas[iter]);
		}
		// summand = summand ^ addend;
		for (iter = 0; iter < max_symb; iter++)
		{
			summand.mas[iter] = XOR(summand.mas[iter], addend.mas[iter]);
		}
		// addend = (carry << 1);
		clear_mem(&addend);
		addend = copy(&carry);
		offset_left(&addend);
		if (oper_sign == 1)
		{
			if (summand.current_count > real_symb)
			{
				summand.mas[real_symb] = 0;
			}
		}
	}

	clear_mem(&addend);
	clear_mem(&carry);
	normalize(&summand);

	return summand;
}

number difference(number *value1, number *value2)
{
	difference_++;
	number b = copy(value2), buff;
	if (value2->mas[value2->current_count - 1])
		additional_code(&b); // nonadditional_code is used
	else
		additional_code(&b);
	// _b = -b
	// a-b = a + (-b) = a + _b
	buff = addition(value1, &b);
	clear_mem(&b);

	return buff;
}

number easy_mult(number *value1, number *value2)
{
	easy_mult_++;
	// TODO: Не использовать numb_to_int, а делать прямо тут
	number result;
	int a = number_to_int(value1);
	int b = number_to_int(value2);

	result = int_to_number(a * b);
	// test commit
	return result;
}

number karatsuba(number *value1, number *value2)
{
	karatsuba_++;
	// printf("Karatsuba\n%d = a_count, %d = b count\n", value1->current_count, value2->current_count);
	// printf("Start karatsuba\n");
	if (value1->current_count < 5 && value2->current_count < 5)
	{
		if (is_zero(value1) || is_zero(value2))
			return int_to_number(0);
		return easy_mult(value1, value2);
	}

	// printf("Karatsuba starts here\n");

	int n;
	int k;
	int iter;
	number res = init(), buff1, buff2;
	number a, b, c, d, p1, p2, t, v1, v2;

	if (value1->current_count < 256 && value2->current_count < 256)
	{
		if (is_zero(value1) || is_zero(value2))
			return int_to_number(0);
		res = multiply_furie(value1, value2);
		// printf("End furie into karatsuba\n");
		return res;
	}
	// k = n/2
	// v1 = a * (2^k) + b, v2 = c * (2^k) + d
	// p1 = b * d
	// p2 = a * c
	// t = (a+b)*(c+d) - p1 - p2
	// res = p2 * 2^n + t * 2^k + p1

	n = max(value1->current_count, value2->current_count) - 1;
	k = n / 2;

	if (n <= 5)
	{
		clear_mem(&res);
#ifdef DEBUG
		printf("Start easyMult\n");

		printf("n = %d\n", n);
		printf("k = %d\n", k);

		printf("v1 = ");
		print_number(value1);
		printf("v2 = ");
		print_number(value2);
#endif // DEBUG
		return easy_mult(value1, value2);
	}
	else
	{
		v1 = copy(value1);
		v2 = copy(value2);

		a = init();
		b = init();
		c = init();
		d = init();

		// TODO тоже memcpy
		for (iter = n - v1.current_count; iter > 0; iter--)
		{
			add_digit(&v1, 0);
		}
		for (iter = n - v2.current_count; iter > 0; iter--)
		{
			add_digit(&v2, 0);
		}

		for (iter = 0; iter < k; iter++)
		{
			add_digit(&b, v1.mas[iter]);
			add_digit(&d, v2.mas[iter]);
		}

		for (iter = k; iter < n; iter++)
		{
			add_digit(&a, v1.mas[iter]);
			add_digit(&c, v2.mas[iter]);
		}

#ifdef DEBUG
		printf("Before all\n");

		printf("n = %d\n", n);
		printf("k = %d\n", k);

		printf("v1 = ");
		print_number(&v1);
		printf("v2 = ");
		print_number(&v2);

		printf("a = ");
		print_number(&a);
		printf("b = ");
		print_number(&b);
		printf("c = ");
		print_number(&c);
		printf("d = ");
		print_number(&d);
#endif // DEBUG

		// printf("Before all\n");

		// printf("n = %d\n", n);
		// printf("k = %d\n", k);

		// printf("v1 = ");
		// print_number_as_is(&v1);
		// printf("v2 = ");
		// print_number_as_is(&v2);

		// printf("a = ");
		// print_number_as_is(&a);
		// printf("b = ");
		// print_number_as_is(&b);
		// printf("c = ");
		// print_number_as_is(&c);
		// printf("d = ");
		// print_number_as_is(&d);

		p1 = karatsuba(&b, &d);
		p2 = karatsuba(&a, &c);
		buff1 = addition(&a, &b);
		buff2 = addition(&c, &d);
		t = karatsuba(&buff1, &buff2);

		clear_mem(&buff1);
		clear_mem(&buff2);
		clear_mem(&a);
		clear_mem(&b);
		clear_mem(&c);
		clear_mem(&d);
		clear_mem(&v1);
		clear_mem(&v2);

		buff1 = copy(&t);
		clear_mem(&t);
		t = difference(&buff1, &p1);
		clear_mem(&buff1);

		buff1 = copy(&t);
		clear_mem(&t);
		t = difference(&buff1, &p2);
		clear_mem(&buff1);

		// printf("Berofe offset\np2 = ");
		// print_number_as_is(&p2);
		// printf("t = ");
		// print_number_as_is(&t);
		// printf("p1 = ");
		// print_number_as_is(&p1);

#ifdef DEBUG
		printf("Berofe offset\np2 = ");
		print_number(&p2);
		printf("t = ");
		print_number(&t);
		printf("p1 = ");
		print_number(&p1);
#endif // DEBUG

		// printf("offset p2\n");
		for (iter = 0; iter < 2 * k; iter++)
		{
			offset_left(&p2);
		}
		// printf("offset t\n");
		for (iter = 0; iter < k; iter++)
		{
			offset_left(&t);
		}
		// printf("end offset\n");

#ifdef DEBUG
		printf("After all\np2 = ");
		print_number(&p2);
		printf("t = ");
		print_number(&t);
		printf("p1 = ");
		print_number(&p1);
#endif // DEBUG

		buff1 = copy(&res);
		clear_mem(&res);
		res = addition(&buff1, &p2);
		clear_mem(&buff1);

		buff1 = copy(&res);
		clear_mem(&res);
		res = addition(&buff1, &p1);
		clear_mem(&buff1);

		buff1 = copy(&res);
		clear_mem(&res);
		res = addition(&buff1, &t);
		clear_mem(&buff1);
#ifdef DEBUG
		printf("res = ");
		print_number(&res);
#endif // DEBUG

		// printf("res = ");
		// print_number_as_is(&res);

		clear_mem(&p1);
		clear_mem(&p2);
		clear_mem(&t);
		////////////////////////////////////////////////////////////////////////////////////////////////
		return res;
	}
}

number multiplication(number *value1, number *value2)
{
	multiplication_++;
	// printf("start multy\n");

	if (is_zero(value1) || is_zero(value2))
		return int_to_number(0);
	number result, a, b;
	// printf("start copy a\n");
	a = copy(value1);
	// printf("start copy b\n");
	b = copy(value2);
	// printf("end copy\n");
	int sign = a.mas[a.current_count - 1] + b.mas[b.current_count - 1];
	if (a.mas[a.current_count - 1])
	{
		additional_code(&a);
	}
	if (b.mas[b.current_count - 1])
	{
		additional_code(&b);
	}
	// printf("go karat\n");

	result = karatsuba(&a, &b); // karatsuba(&a, &b);
	// number result2 = karatsuba(&a, &b);
	// if (!is_equal(&result, &result2))
	// {
	// 	char num[100] = "";
	// 	s//printf(num, "%d", 42);
	// 	_log("=====");
	// 	_log("Furie");
	// 	debug_log(&result);
	// 	s//printf(num, "Cur-count:%d, size:%d\n", result.current_count, result.size);
	// 	_log(num);
	// 	num[0] = '\0';
	// 	_log("Karatsuba");
	// 	debug_log(&result2);
	// 	s//printf(num, "Cur-count:%d, size:%d\n", result2.current_count, result2.size);
	// 	_log(num);
	// 	_log("A");
	// 	debug_log(&a);
	// 	_log("B");
	// 	debug_log(&b);
	// 	_log("!!!!!");
	// 	FILE *file;
	// 	file = fopen("debug.txt", "w");

	// 	if (file == NULL)
	// 	{
	// 		_log("Error while reading file (write):");
	// 		_log("debug.txt");
	// 		exit(FILE_OPEN_FAILURE);
	// 	}
	// 	save_num_to_filer(file, &result);
	// 	save_num_to_filer(file, &result2);
	// 	save_num_to_filer(file, &a);
	// 	save_num_to_filer(file, &b);
	// 	fclose(file);
	// 	exit(FAILURE);
	// }
	// printf("clear_mem a\n");
	clear_mem(&a);
	// printf("clear_mem b\n");
	clear_mem(&b);
	// printf("clear_mem done\n");

	if (sign == 1)
	{
		// printf("go add_code\n");
		additional_code(&result);
		// printf("end add_code\n");
	}
	// printf("go normal\n");
	normalize(&result);
	// printf("end normal\n");

	return result;
}

number division_with_module(number *value1, number *value2, number *ost)
{
	division_with_module_++;
	// TODO темный лес
	// printf("Start divMod\n");
	// debug_log(value1);
	// debug_log(value2);
	number mod = int_to_number(0), rem = copy(value1), sub = copy(value2);
	number add = int_to_number(1);
	number buff;
	short shifts = 1;

	if (is_zero(value2))
	{
		_log("Divider was 0 in division_with_module() function");
		exit(FAILURE);
	}
	if (value2->mas[value2->current_count - 1])
	{
		clear_mem(&mod);
		clear_mem(&rem);

		buff = copy(value2);
		additional_code(&buff); // nonadditional_code is used
		mod = division_with_module(value1, &buff, &rem);
		clear_mem(&buff);

		*ost = rem;
		additional_code(&mod);
		clear_mem(&add);
		clear_mem(&sub);
		return mod;
	}
	if (value1->mas[value1->current_count - 1])
	{
		clear_mem(&mod);
		clear_mem(&rem);

		// b - ( |a| % b )
		buff = copy(value1);
		additional_code(&buff);							 // nonadditional_code is used
		mod = division_with_module(&buff, value2, &rem); // ost = ( |a| % b )
		clear_mem(&buff);

		if (is_zero(&rem))
		{
			buff = int_to_number(0);
			clear_mem(&rem);
			rem = copy(&buff);
			clear_mem(&buff);
		}
		else
		{
			buff = difference(value2, &rem);
			clear_mem(&rem);
			rem = copy(&buff);
			clear_mem(&buff);

			buff = addition(&mod, &add);
			clear_mem(&mod);
			mod = copy(&buff);
			clear_mem(&buff);
		}
		*ost = rem;
		additional_code(&mod);
		clear_mem(&add);
		clear_mem(&sub);
		return mod;
	}

	buff = difference(&sub, &rem);
	// debug_log(&buff);
	// debug_log(&sub);
	// debug_log(&rem);
	// printf("First cycle\n");
	while (buff.mas[buff.current_count - 1])
	{
		offset_left(&sub);
		offset_left(&add);
		clear_mem(&buff);
		buff = difference(&sub, &rem);
		shifts++;
	}
	clear_mem(&buff);
	// printf("Second cycle\n");
	while (shifts)
	{
		buff = difference(&rem, &sub);
		while (!buff.mas[buff.current_count - 1])
		{
			clear_mem(&buff);
			buff = difference(&rem, &sub);
			clear_mem(&rem);
			rem = copy(&buff);
			clear_mem(&buff);

			buff = addition(&add, &mod);
			clear_mem(&mod);
			mod = copy(&buff);
			clear_mem(&buff);

			buff = difference(&rem, &sub);
		}
		clear_mem(&buff);
		offset_right(&sub);
		offset_right(&add);
		shifts--;
	}

	clear_mem(&sub);
	clear_mem(&add);

	normalize(&rem);
	normalize(&mod);
	*ost = rem;
	return mod;
}

number module_pow(number *a, number *t, number *b)
{
	module_pow_++;
	// printf("=====================================\nD is ");
	// print_number_as_is(t);
	// TODO тоже темный лес, но светлее
	number d, ost, iterator = init(), buff, buff2, buff3;

	add_digit(&iterator, 1);
	buff3 = division_with_module(a, b, &d);
	clear_mem(&buff3);
	ost = copy(&d);
	// printf("=====================================\nPassed divMod\n");
	if (is_zero(&d))
	{
		clear_mem(&d);
		clear_mem(&iterator);
		return ost;
	}
	else
	{
		clear_mem(&iterator);
		iterator = copy(t);

		clear_mem(&ost);
		ost = int_to_number(1);

		buff2 = copy(a);
		while (!is_zero(&iterator))
		{
			if (iterator.mas[0] == 1)
			{
				buff = multiplication(&ost, &buff2);
				clear_mem(&ost);
				buff3 = division_with_module(&buff, b, &ost);
				clear_mem(&buff3);
				clear_mem(&buff);
			}
			buff = multiplication(&buff2, &buff2);
			clear_mem(&buff2);
			buff3 = division_with_module(&buff, b, &buff2);
			clear_mem(&buff3);
			clear_mem(&buff);

			offset_right(&iterator);
		}
		buff = copy(&ost);
		clear_mem(&ost);
		buff3 = division_with_module(&buff, b, &ost);
		clear_mem(&buff);
		clear_mem(&buff2);
		clear_mem(&buff3);
	}
	clear_mem(&d);
	clear_mem(&iterator);
	normalize(&ost);
	return ost;
}

number euclide_algorithm(number *value1, number *value2)
{
	number buff, a, b, mod;

	a = copy(value1);
	if (a.mas[a.current_count - 1])
	{
		additional_code(&a); // nonadditional_code is used
	}
	b = copy(value2);
	if (b.mas[b.current_count - 1])
	{
		additional_code(&b); // nonadditional_code is used
	}
	buff = difference(&a, &b);
	if (buff.mas[buff.current_count - 1])
	{
		clear_mem(&buff);
		clear_mem(&a);
		clear_mem(&b);
		a = copy(value2);
		b = copy(value1);
	}
	else
	{
		clear_mem(&buff);
		clear_mem(&a);
		clear_mem(&b);
		a = copy(value1);
		b = copy(value2);
	}
	if (a.mas[a.current_count - 1])
	{
		additional_code(&a); // nonadditional_code is used
	}
	if (b.mas[b.current_count - 1])
	{
		additional_code(&b); // nonadditional_code is used
	}
	// a = b * q_0 + r_1
	buff = division_with_module(&a, &b, &mod);
	clear_mem(&buff);

	if (!is_zero(&mod))
	{
		buff = euclide_algorithm(&b, &mod);
		clear_mem(&mod);
		clear_mem(&a);
		clear_mem(&b);
		normalize(&buff);
		return buff;
	}
	else
	{
		clear_mem(&mod);
		clear_mem(&a);
		normalize(&b);
		return b;
	}
}

number euclide_algorithm_modifyed(number *value1, number *value2, number *values)
{
	// TODO ...
	number buff, a, b, mod, div, GCD;
	number _a, _b, _c, _d;

	a = copy(value1);
	b = copy(value2);
	buff = difference(&a, &b);
	if (buff.mas[buff.current_count - 1])
	{
		clear_mem(&buff);
		clear_mem(&a);
		clear_mem(&b);
		a = copy(value2);
		b = copy(value1);
	}
	else
	{
		clear_mem(&buff);
		clear_mem(&a);
		clear_mem(&b);
		a = copy(value1);
		b = copy(value2);
	}
	// a = b * q_0 + r_1

	div = division_with_module(&a, &b, &mod);
	if (!is_zero(&mod))
	{
		buff = euclide_algorithm_modifyed(&b, &mod, values);
		GCD = copy(&buff);
		clear_mem(&buff);

		_a = copy(&values[0]);
		_b = copy(&values[1]);
		_c = copy(&values[2]);
		_d = copy(&values[3]);

		clear_mem(&values[0]);
		values[0] = copy(&_b);

		buff = multiplication(&_b, &div);
		clear_mem(&values[1]);
		values[1] = difference(&_a, &buff);
		clear_mem(&buff);

		clear_mem(&values[2]);
		values[2] = copy(&_d);

		buff = multiplication(&_d, &div);
		clear_mem(&values[3]);
		values[3] = difference(&_c, &buff);
		clear_mem(&buff);

		clear_mem(&_a);
		clear_mem(&_b);
		clear_mem(&_c);
		clear_mem(&_d);
		clear_mem(&div);
		clear_mem(&mod);
		clear_mem(&a);
		clear_mem(&b);

		normalize(&GCD);

		return GCD;
	}
	else
	{
		GCD = copy(&b);

		_a = copy(&values[0]);
		_b = copy(&values[1]);
		_c = copy(&values[2]);
		_d = copy(&values[3]);

		clear_mem(&values[0]);
		values[0] = copy(&_b);

		buff = multiplication(&_b, &div);
		clear_mem(&values[1]);
		values[1] = difference(&_a, &buff);
		clear_mem(&buff);

		clear_mem(&values[2]);
		values[2] = copy(&_d);

		buff = multiplication(&_d, &div);
		clear_mem(&values[3]);
		values[3] = difference(&_c, &buff);
		clear_mem(&buff);

		clear_mem(&_a);
		clear_mem(&_b);
		clear_mem(&_c);
		clear_mem(&_d);
		clear_mem(&div);
		clear_mem(&mod);
		clear_mem(&a);
		clear_mem(&b);
		normalize(&GCD);
		return GCD;
	}
}

number generate_random(int bit_count)
{
	int i;
	number res = init();
	for (i = 0; i < bit_count - 1; i++)
	{
		add_digit(&res, rand() % 2);
	}
	return res;
}

BOOL Millers_method(number *value)
{
	number t, buff, buff2, a, N_minus_1;
	BOOL fl1, fl2;
	number step = int_to_number(1);
	number step2 = int_to_number(2);
	number N_3 = int_to_number(3);
	int n = MILLERS_METHOD_ITERATIONS_NUMBER;
	int s = 0;
	int i, k, j; // iterators

	if (is_equal(value, &step2))
	{
		clear_mem(&step);
		clear_mem(&step2);
		clear_mem(&N_3);
		return TRUE;
	}

	if (is_equal(value, &N_3))
	{
		clear_mem(&step);
		clear_mem(&step2);
		clear_mem(&N_3);
		return TRUE;
	}

	if (!value->mas[0])
	{
		clear_mem(&step);
		clear_mem(&step2);
		clear_mem(&N_3);
		return FALSE;
	}

	// N-1 = 2^s * t, t - нечетно

	N_minus_1 = difference(value, &step);
	t = copy(&N_minus_1);

	while (t.mas[0] == 0)
	{
		s++;
		offset_right(&t);
	}

	srand((unsigned int)time(NULL));

	for (i = 0; i < n; i++)
	{
		printf("i is %d\n", i);
		fl1 = TRUE;
		fl2 = TRUE;

		a = generate_random(value->current_count - 1);
		buff = difference(&a, &step2);
		while (buff.mas[buff.current_count - 1])
		{
			clear_mem(&buff);
			clear_mem(&a);
			a = generate_random(value->current_count - 1);
			buff = difference(&a, &step2);
		}
		clear_mem(&buff);

		buff = euclide_algorithm(value, &a);
		if (!is_equal(&buff, &step)) // свойство 1
		{
			fl1 = FALSE;
		}
		clear_mem(&buff);

		buff = module_pow(&a, &t, value);
		if (!is_equal(&buff, &step)) // свойство 2
		{
			fl2 = FALSE;
			for (k = 1; k <= s; k++)
			{
				clear_mem(&buff);
				buff2 = copy(&t);
				for (j = 0; j < k - 1; j++)
				{
					offset_left(&buff2);
				}

				buff = module_pow(&a, &buff2, value);
				clear_mem(&buff2);

				if (is_equal(&buff, &N_minus_1))
				{
					// printf("= is %d\n", is_equal(&buff, &N_minus_1));
					fl2 = TRUE;
				}
			}
		}
		clear_mem(&buff);

		if (!fl1 || !fl2)
		{
			clear_mem(&step);
			clear_mem(&step2);
			clear_mem(&N_3);
			clear_mem(&t);
			clear_mem(&a);
			clear_mem(&N_minus_1);
			return FALSE;
		}

		clear_mem(&a);
	}

	clear_mem(&step);
	clear_mem(&step2);
	clear_mem(&N_3);
	clear_mem(&t);
	clear_mem(&N_minus_1);
	return TRUE;
}