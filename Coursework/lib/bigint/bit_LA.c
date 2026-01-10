//
// Created by Georgul on 02.11.2021.
//
#include "bit_LA.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

#define XOR(a, b) a ^ b

#define NOT(a) !a

#define AND(a, b) a &b

// FIXME алгоритмическая
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
	int num = 1;
	*ln_count = 0;

	*input = (char *)malloc(value->current_count);
	if (*input == NULL) return MEMORY_ALLOCATION_FAILURE;

	memcpy(*input, value->mas, value->current_count);
	*count = value->current_count;

	do {
		num <<= 1;
		*ln_count += 1;
	} while (*count > num);

	*input = (char *)realloc(*input, num);
	if (*input == NULL) return MEMORY_ALLOCATION_FAILURE;

	for (; *count < num; (*count)++)
		(*input)[(*count)] = 0;

	return SUCCESS;
}

number multiply_furie(number *value_1, number *value_2)
{
	if (is_zero(value_1) || is_zero(value_2))
		return int_to_number(0);
	int i; // iterator

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
	int len, ln_count, len_1, ln_count_1, len_2, ln_count_2;
	char *mas_1, *mas_2;
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
	normalize(&result);
	return result;
}

#pragma endregion furie

void additional_code(number *value)
{
	// additional_code_++;
	if (!is_zero(value))
	{
		uint8_t addit_digit = 1;
		char *iter;

		// FIXME машинно-зависимая
		// FIXME машинно-независимая (цикл)
		for (iter = value->mas; iter < value->mas + value->current_count; iter++)
		{
			*iter = NOT(*iter);
			*iter = XOR(*iter, addit_digit);
			if (*iter)
				addit_digit = 0;
		}
		// for (iter = 0; iter < value->current_count; iter++)
		// {
		// 	value->mas[iter] = NOT(value->mas[iter]);
		// 	value->mas[iter] = XOR(value->mas[iter], addit_digit);
		// 	if (value->mas[iter])
		// 		addit_digit = 0;
		// }
	}
}

number init()
{
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
	number result = init();
	clear_mem(&result);
	result.mas = (uint8_t *)malloc(sizeof(uint8_t) * (value->size));
	if (result.mas == NULL)
	{
		_log("Memory allocation failure in add_digit() function (buffer)");
		exit(MEMORY_ALLOCATION_FAILURE);
	}
	// FIXME машинно-независимая + алгоритм?
	memcpy(result.mas, value->mas, value->size);
	result.current_count = value->current_count;
	result.size = value->size;
	return result;
}

number int_to_number(int value)
{
	int ostatok;
	number result = init();

	if (value < 0)
	{
		value *= -1;
		while (value > 0)
		{
			ostatok = value & 1;
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
			ostatok = value & 1;
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
	// FIXME машинно-независимая + алгоритм?
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
	if (value == NULL)
		return;
	if (value->mas != NULL)
	{
		free(value->mas);
		value->mas = NULL;
	}
}

void add_digit(number *object, uint8_t value)
{
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
		// FIXME машинно-независимая
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
	if (object->current_count == 2)
	{
		clear_mem(object);
		*object = int_to_number(0);
	}
	else
	{
		reverse(object);
		object->mas[object->current_count - 2] = object->mas[object->current_count - 1];
		object->current_count -= 1;
		reverse(object);

		normalize(object);
	}
}

void offset_left(number *value)
{
	reverse(value);
	add_digit(value, 0);
	swap(value->mas[value->current_count - 2], value->mas[value->current_count - 1]);
	reverse(value);
	normalize(value);
}

void reverse(number *value)
{
	number prom = init();
	uint8_t *iter; // iterator
	for (iter = value->mas + value->current_count - 2; iter >= value->mas; iter--)
	{
		add_digit(&prom, *iter);
	}
	// FIXME машинно-независимая
	memcpy(value->mas, prom.mas, prom.current_count - 1);
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
	number b = copy(value2), buff;
	if (value2->mas[value2->current_count - 1])
		additional_code(&b); // nonadditional_code is used
	else
		additional_code(&b);
	buff = addition(value1, &b);
	clear_mem(&b);

	return buff;
}

number easy_mult(number *value1, number *value2)
{
	number result;
	int a = number_to_int(value1);
	int b = number_to_int(value2);

	result = int_to_number(a * b);
	// test commit
	return result;
}

number karatsuba(number *value1, number *value2)
{
	if (value1->current_count < 5 && value2->current_count < 5)
	{
		if (is_zero(value1) || is_zero(value2))
			return int_to_number(0);
		return easy_mult(value1, value2);
	}

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

		for (iter = 0; iter < 2 * k; iter++)
		{
			offset_left(&p2);
		}
		for (iter = 0; iter < k; iter++)
		{
			offset_left(&t);
		}
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
		clear_mem(&p1);
		clear_mem(&p2);
		clear_mem(&t);
		return res;
	}
}

number multiplication(number *value1, number *value2)
{
	if (is_zero(value1) || is_zero(value2))
		return int_to_number(0);
	number result, a, b;
	a = copy(value1);
	b = copy(value2);
	int sign = a.mas[a.current_count - 1] + b.mas[b.current_count - 1];
	if (a.mas[a.current_count - 1])
	{
		additional_code(&a);
	}
	if (b.mas[b.current_count - 1])
	{
		additional_code(&b);
	}

	result = karatsuba(&a, &b);
	clear_mem(&a);
	clear_mem(&b);

	if (sign == 1)
	{
		additional_code(&result);
	}
	normalize(&result);

	return result;
}

number division_with_module(number *value1, number *value2, number *ost)
{
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
	while (buff.mas[buff.current_count - 1])
	{
		offset_left(&sub);
		offset_left(&add);
		clear_mem(&buff);
		buff = difference(&sub, &rem);
		// FIXME ассемблерная
		shifts++;
		// asm(
		// 	"movl %0, %%eax\n"
		// 	"addl $1, %%eax\n"
		// 	"movl %%eax, %0\n"
		// 	: "=m"(shifts));
	}
	clear_mem(&buff);
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
	number d, ost, iterator = init(), buff, buff2, buff3;

	add_digit(&iterator, 1);
	buff3 = division_with_module(a, b, &d);
	clear_mem(&buff3);
	ost = copy(&d);
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
