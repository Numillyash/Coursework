//
// Created by Georgul on 02.11.2021.
//
#include "bit_LA.h"

inline uint8_t XOR(uint8_t a, uint8_t b)
{
	return a ^ b;
}

inline uint8_t NOT(uint8_t a)
{
	return (a) ? 0 : 1;
}

inline uint8_t AND(uint8_t a, uint8_t b)
{
	return a & b;
}

void additional_code(number* value)
{
	if (!is_zero(value)) {
		uint8_t addit_digit = 1;
		int iter;

		for (iter = 0; iter < value->current_count; iter++)
		{
			value->mas[iter] = NOT(value->mas[iter]);
		}

		for (iter = 0; iter < value->current_count; iter++)
		{
			value->mas[iter] = XOR(value->mas[iter], addit_digit);
			if (value->mas[iter])
				addit_digit = 0;
		}
	}
}

void nonadditional_code(number* value)
{
	if (!is_zero(value)) {
		uint8_t addit_digit = 1;
		int iter;

		for (iter = 0; iter < value->current_count; iter++)
		{
			value->mas[iter] = NOT(value->mas[iter]);
		}

		for (iter = 0; iter < value->current_count; iter++)
		{
			value->mas[iter] = XOR(value->mas[iter], addit_digit);
			if (value->mas[iter])
				addit_digit = 0;
		}
	}
}

number init() {
	number result = { 1, 1 };
	result.mas = (uint8_t*)malloc(sizeof(uint8_t));
	if (result.mas == NULL)
	{
		exit(MEMORY_ALLOCATION_FAILURE);
	}
	result.mas[0] = 0;
	return result;
}

number copy(number* value) {
	number result = init();
	for (int i = 0; i < value->current_count - 1; i++)
		add_digit(&result, value->mas[i]);
	result.mas[result.current_count - 1] = value->mas[value->current_count - 1];
	return result;
}

number int_to_number(int value) {
	number result = init();
	if (value < 0)
	{
		value *= -1;
		int ostatok;
		while (value > 0) {
			ostatok = value % NUMBER_SYSTEM_BASE;
			add_digit(&result, ostatok);
			value /= NUMBER_SYSTEM_BASE;
		}
		if (result.current_count == 0)
			add_digit(&result, 0);
		additional_code(&result);
	}
	else
	{
		int ostatok;
		while (value > 0) {
			ostatok = value % NUMBER_SYSTEM_BASE;
			add_digit(&result, ostatok);
			value /= NUMBER_SYSTEM_BASE;
		}
		if (result.current_count == 0)
			add_digit(&result, 0);
	}

	return result;
}

void normalize(number* value) {
	number result = init();
	int end = value->current_count - 2;
	if (value->mas[value->current_count - 1])
	{
		for (int i = value->current_count - 2; i > 0; i--) {
			end = i;
			if (value->mas[i] == 0 || value->mas[i - 1] == 0) {
				break;
			}
		}
	}
	else
	{
		for (int i = value->current_count - 2; i >= 0; i--) {
			end = i;
			if (value->mas[i] != 0) {
				break;
			}
		}
	}
	for (int i = 0; i <= end; i++) {
		add_digit(&result, value->mas[i]);
	}
	result.mas[result.current_count - 1] = value->mas[value->current_count - 1];
	value->current_count = 1;
	value->size = 1;
	free(value->mas);
	value->mas = (uint8_t*)malloc(sizeof(uint8_t));
	if (value->mas == NULL)
	{
		exit(MEMORY_ALLOCATION_FAILURE);
	}
	for (int i = 0; i <= end; i++) {
		add_digit(value, result.mas[i]);
	}
	value->mas[value->current_count - 1] = result.mas[result.current_count - 1];
	clear_mem(&result);
}

void clear_mem(number* value)
{
	free(value->mas);
	value->mas = NULL;
}

void add_digit(number* object, uint8_t value) {
	int iter;
	uint8_t* buff;

	if (object->current_count < object->size) {
		object->mas[object->current_count] = object->mas[object->current_count - 1];
		object->mas[object->current_count - 1] = value;
		object->current_count += 1;
	}
	else {
		buff = (uint8_t*)malloc(sizeof(uint8_t) * (object->current_count));
		if (buff == NULL)
			exit(MEMORY_ALLOCATION_FAILURE);
		for (iter = 0; iter < object->current_count; ++iter)
		{
			buff[iter] = object->mas[iter];
		}
		clear_mem(object);
		object->mas = (uint8_t*)malloc(sizeof(uint8_t) * (object->size) * 2);
		if (object->mas == NULL)
			exit(MEMORY_ALLOCATION_FAILURE);
		for (iter = 0; iter < object->current_count; ++iter)
		{
			object->mas[iter] = buff[iter];
		}
		free(buff);
		object->mas[object->current_count] = object->mas[object->current_count - 1];
		object->mas[object->current_count - 1] = value;
		object->current_count += 1;
		object->size *= 2;
	}
}

void offset_right(number* object)
{
	reverse(object);
	object->current_count -= 1;
	reverse(object);
	normalize(object);
}

void offset_left(number* object)
{
	reverse(object);
	add_digit(object, 0);
	swap(object->mas[object->current_count - 2], object->mas[object->current_count - 1]);
	reverse(object);
	normalize(object);
}

void reverse(number* value)
{
	number prom = init();
	for (int i = value->current_count - 1; i >= 0; i--)
	{
		add_digit(&prom, value->mas[i]);
	}
	for (int i = 0; i < prom.current_count - 1; i++)
	{
		value->mas[i] = prom.mas[i];
	}
	clear_mem(&prom);
}

void print_number(number* value) {
	printf("%d ", (int)value->mas[value->current_count - 1]);
	for (int i = value->current_count - 2; i >= 0; i--)
	{
		printf("%d", (int)value->mas[i]);
		if (i % 8 == 0)
			printf(" ");
	}
	printf("\n");
}

BOOL is_zero(number* object)
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

BOOL is_equal(number* value1, number* value2)
{
	short iterator = 0;

	if (value1->current_count != value2->current_count)
		return FALSE;
	for (iterator = 0; iterator < value1->current_count; iterator++)
	{
		if (value1->mas[iterator] != value2->mas[iterator])
			return FALSE;
	}

	return TRUE;
}

number addition(number* value1, number* value2) {
	number summand, addend;
	number carry = init();
	int real_symb = 0;
	int oper_sign = 0;
	int iter = 0;
	int max_symb = 0;

	normalize(value1); normalize(value2);

	if ((value1->current_count) >= (value2->current_count)) {
		summand = copy(value1);
		addend = copy(value2);
	}
	else {
		summand = copy(value2);
		addend = copy(value1);
	}
	oper_sign = summand.mas[summand.current_count - 1] + addend.mas[addend.current_count - 1];
	real_symb = max(summand.current_count, addend.current_count);

	while (!is_zero(&addend))
	{
		max_symb = max(summand.current_count, addend.current_count);
		max_symb = max(max_symb, carry.current_count);
		if (max_symb == 2) { max_symb++; real_symb++; }

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

		//carry = (summand & addend);
		for (iter = 0; iter < max_symb; iter++)
		{
			carry.mas[iter] = AND(summand.mas[iter], addend.mas[iter]);
		}

		//summand = summand ^ addend;
		for (iter = 0; iter < max_symb; iter++)
		{
			summand.mas[iter] = XOR(summand.mas[iter], addend.mas[iter]);
		}

		//addend = (carry << 1);
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

number difference(number* value1, number* value2) {
	number b = copy(value2);
	if (value2->mas[value2->current_count - 1])
	{
		nonadditional_code(&b);
	}
	else
		additional_code(&b);
	return addition(value1, &b);
}

number easy_mult(number* value1, number* value2)
{
	number result = init(), buff;
	number a = copy(value1);
	number b = copy(value2);
	while (!is_zero(&b)) {
		if (b.mas[0])
		{
			buff = addition(&result, &a);
			clear_mem(&result);
			result = copy(&buff);
			clear_mem(&buff);
		}
		offset_right(&b);
		offset_left(&a);
	}
	clear_mem(&a);
	clear_mem(&b);
	return result;
}

number karatsuba(number* value1, number* value2)
{
	int n; int k; int iter;
	number res = init(), buff1, buff2;
	number a, b, c, d, p1, p2, t, v1, v2;

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
		v1 = copy(value1); v2 = copy(value2);

		a = init(); b = init(); c = init(); d = init();

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

		p1 = karatsuba(&b, &d);
		p2 = karatsuba(&a, &c);
		buff1 = addition(&a, &b);
		buff2 = addition(&c, &d);
		t = karatsuba(&buff1, &buff2);
		clear_mem(&buff1); clear_mem(&buff2); clear_mem(&a); clear_mem(&b); clear_mem(&c); clear_mem(&d); clear_mem(&v1); clear_mem(&v2);

		buff1 = copy(&t);
		clear_mem(&t);
		t = difference(&buff1, &p1);
		clear_mem(&buff1);

		buff1 = copy(&t);
		clear_mem(&t);
		t = difference(&buff1, &p2);
		clear_mem(&buff1);

#ifdef DEBUG
		printf("Berofe offset\np2 = ");
		print_number(&p2);
		printf("t = ");
		print_number(&t);
		printf("p1 = ");
		print_number(&p1);
#endif // DEBUG

		for (iter = 0; iter < 2 * k; iter++)
		{
			offset_left(&p2);
		}

		for (iter = 0; iter < k; iter++)
		{
			offset_left(&t);
		}

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
		return res;
	}
}

number multiplication(number* value1, number* value2) {
	number result = init(), a, b;
	a = copy(value1); b = copy(value2);
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
	clear_mem(&a); clear_mem(&b);

	if (sign == 1) {
		additional_code(&result);
	}

	return result;
}

//fix
number division_with_remainder(number* value1, number* value2, number* ost)
{
	number quot = int_to_number(0), rem = copy(value1), sub = copy(value2);
	number add = int_to_number(1);
	number buff;
	short shifts = 1;

	if (is_zero(value2))
	{
		exit(FAILURE);
	}
	if (value2->mas[value2->current_count - 1])
	{
		buff = copy(value2);
		nonadditional_code(&buff);
		number _ost;
		number result = division_with_remainder(value1, &buff, &_ost);
		clear_mem(&buff);

		*ost = _ost;
		additional_code(&result);
		clear_mem(&sub);
		clear_mem(&add);
		clear_mem(&rem);
		clear_mem(&quot);
		return result;
	}
	if (value1->mas[value1->current_count - 1])
	{
		// b - ( |a| % b )
		buff = copy(value1);
		nonadditional_code(&buff);
		number _ost;
		number result = division_with_remainder(&buff, value2, &_ost); // ost = ( |a| % b )
		clear_mem(&buff);

		if (is_zero(&_ost))
		{
			buff = int_to_number(0);
			clear_mem(&_ost);
			_ost = copy(&buff);
			clear_mem(&buff);
		}
		else {
			buff = difference(value2, &_ost);
			clear_mem(&_ost);
			_ost = copy(&buff);
			clear_mem(&buff);

			buff = addition(&result, &add);
			clear_mem(&result);
			result = copy(&buff);
			clear_mem(&buff);
		}
		*ost = _ost;
		additional_code(&result);
		clear_mem(&sub);
		clear_mem(&add);
		clear_mem(&rem);
		clear_mem(&quot);
		return result;
	}

	*ost = init();

	while ((buff = difference(&sub, &rem)).mas[buff.current_count-1])
	{
		offset_left(&sub);
		offset_left(&add);
		clear_mem(&buff);
		shifts++;
	}
	clear_mem(&buff);
	while (shifts)
	{
		while (!(buff = difference(&rem, &sub)).mas[buff.current_count - 1])
		{
			clear_mem(&buff);
			buff = difference(&rem, &sub);
			clear_mem(&rem);
			rem = copy(&buff);
			clear_mem(&buff);

			buff = addition(&add, &quot);
			clear_mem(&quot);
			quot = copy(&buff);
			clear_mem(&buff);
		}
		offset_right(&sub);
		offset_right(&add);
		shifts--;
	}

	clear_mem(&sub);
	clear_mem(&add);

	*ost = rem;
	return quot;
}

/*

number division(number* value1, number* value2)
{
	number osn = int_to_number(256);
	number quot = int_to_number(0), rem = copy(value1), sub = copy(value2);
	number add = int_to_number(1);
	number buff;
	short shifts = 1;

	while ((buff = difference(&sub, &rem)).negative == -1)
	{
		clear_mem(&buff);
		buff = multiplication(&sub, &osn);
		clear_mem(&sub);
		sub = copy(&buff);

		clear_mem(&buff);
		buff = multiplication(&add, &osn);
		clear_mem(&add);
		add = copy(&buff);
		clear_mem(&buff);
		shifts++;
	}
	clear_mem(&buff);
	while (shifts)
	{
		while ((buff = difference(&rem, &sub)).negative != -1)
		{
			clear_mem(&buff);
			buff = difference(&rem, &sub);
			clear_mem(&rem);
			rem = copy(&buff);
			clear_mem(&buff);

			buff = addition(&add, &quot);
			clear_mem(&quot);
			quot = copy(&buff);
			clear_mem(&buff);
		}
		offset_right(&sub);
		offset_right(&add);
		shifts--;
	}

	clear_mem(&osn);
	clear_mem(&sub);
	clear_mem(&add);

	return quot;
}
*/