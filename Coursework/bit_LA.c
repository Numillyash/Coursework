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
		_log("Memory allocation failure in init() function");
		exit(MEMORY_ALLOCATION_FAILURE);
	}
	result.mas[0] = 0;
	return result;
}

number copy(number* value) {
	number result = init();
	int i; // iterator
	for (i = 0; i < value->current_count - 1; i++)
		add_digit(&result, value->mas[i]);
	result.mas[result.current_count - 1] = value->mas[value->current_count - 1];
	return result;
}

number int_to_number(int value) {
	int ostatok;
	number result = init();
	if (value < 0)
	{
		value *= -1;
		while (value > 0) {
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
		while (value > 0) {
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

int number_to_int(number* value) 
{
	int res = 0, i, zn = 1;
	int x = 1; // степень двойки

	if (value->mas[value->current_count - 1])
	{
		zn = -1;
		nonadditional_code(value);
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

void normalize(number* value) {
	number result = init();
	int i; //iterator
	int end = value->current_count - 2;
	if (value->mas[value->current_count - 1])
	{
		for (i = value->current_count - 2; i > 0; i--) {
			end = i;
			if (value->mas[i] == 0 || value->mas[i - 1] == 0) {
				break;
			}
		}
	}
	else
	{
		for (i = value->current_count - 2; i >= 0; i--) {
			end = i;
			if (value->mas[i] != 0) {
				break;
			}
		}
	}
	for (i = 0; i <= end; i++) {
		add_digit(&result, value->mas[i]);
	}
	result.mas[result.current_count - 1] = value->mas[value->current_count - 1];
	value->current_count = 1;
	value->size = 1;
	free(value->mas);
	value->mas = (uint8_t*)malloc(sizeof(uint8_t));
	if (value->mas == NULL)
	{
		_log("Memory allocation failure in normalize() function");
		exit(MEMORY_ALLOCATION_FAILURE);
	}
	for (i = 0; i <= end; i++) {
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
		{
			_log("Memory allocation failure in add_digit() function (buffer)");
			exit(MEMORY_ALLOCATION_FAILURE);
		}
		for (iter = 0; iter < object->current_count; ++iter)
		{
			buff[iter] = object->mas[iter];
		}
		clear_mem(object);
		object->mas = (uint8_t*)malloc(sizeof(uint8_t) * (object->size) * 2);
		if (object->mas == NULL)
		{
			_log("Memory allocation failure in add_digit() function (object)");
			exit(MEMORY_ALLOCATION_FAILURE);
		}
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

void print_number_as_is(number* value) {
	int i; //iterator
	for (i = 0; i <= value->current_count - 2; i++)
	{
		printf("%d", (int)value->mas[i]);
		if (i % 4 == 3)
			printf(" ");
	}
	printf(" %d", (int)value->mas[value->current_count - 1]);
	printf("\n");
}

void print_number_decimal(number* value) {
	int y; //iterator
	int x = 1; // stepen 2
	int s = 0; // summa
	if (value->mas[value->current_count - 1])
	{
		printf("-");
		nonadditional_code(value);
		
		for (y = 0; y < value->current_count - 1; y++)
		{
			s += value->mas[y] * x;
			x <<= 1;
		}
		printf("%d\n", s);
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

void print_number(number* value) {
	int i; //iterator
	printf("%d ", (int)value->mas[value->current_count - 1]);
	while (((value->current_count) - 1) % 4 != 0)
	{
		add_digit(value, 0);
	}
	for (i = value->current_count - 2; i >= 0; i--)
	{
		printf("%d", (int)value->mas[i]);
		if (i % 4 == 0)
			printf(" ");
	}
	normalize(value);
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
	number b = copy(value2), buff;
	if (value2->mas[value2->current_count - 1])
		nonadditional_code(&b);
	else
		additional_code(&b);

	buff = addition(value1, &b);
	clear_mem(&b);

	return buff;
}

number easy_mult(number* value1, number* value2)
{
	number result = init();
	int a = number_to_int(value1);
	int b = number_to_int(value2);
	
	result = int_to_number(a*b);

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
		clear_mem(&p1);
		clear_mem(&p2);
		clear_mem(&t);
		////////////////////////////////////////////////////////////////////////////////////////////////
		return res;
	}
}

number multiplication(number* value1, number* value2) {
	number result, a, b;
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
	normalize(&result);
	return result;
}

number division_with_module(number* value1, number* value2, number* ost)
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
		nonadditional_code(&buff);
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
		nonadditional_code(&buff);
		mod = division_with_module(&buff, value2, &rem); // ost = ( |a| % b )
		clear_mem(&buff);

		if (is_zero(&rem))
		{
			buff = int_to_number(0);
			clear_mem(&rem);
			rem = copy(&buff);
			clear_mem(&buff);
		}
		else {
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
		shifts++;
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

number module_pow(number* a, number* t, number* b)
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
			if (iterator.mas[0] % 2 == 1)
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

number euclide_algorithm(number* value1, number* value2)
{
	number buff, a, b, mod;

	a = copy(value1);
	if (a.mas[a.current_count - 1])
	{
		nonadditional_code(&a);
	}
	b = copy(value2);
	if (b.mas[b.current_count - 1])
	{
		nonadditional_code(&b);
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
		nonadditional_code(&a);
	}
	if (b.mas[b.current_count - 1])
	{
		nonadditional_code(&b);
	}
	//a = b * q_0 + r_1                          
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

number euclide_algorithm_modifyed(number* value1, number* value2, number* values)
{
	number buff, a, b, mod, div, GCD;
	number _a, _b, _c, _d;

	a = copy(value1);
	if (a.mas[a.current_count - 1])
	{
		nonadditional_code(&a);
	}
	b = copy(value2);
	if (b.mas[b.current_count - 1])
	{
		nonadditional_code(&b);
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
		nonadditional_code(&a);
	}
	if (b.mas[b.current_count - 1])
	{
		nonadditional_code(&b);
	}
	//a = b * q_0 + r_1                          
	div = division_with_module(&a, &b, &mod);
	//clear_mem(&buff);

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

BOOL Millers_method(number* value)
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

	//N-1 = 2^s * t, t - нечетно

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
		if (!is_equal(&buff, &step)) //свойство 1
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
					//printf("= is %d\n", is_equal(&buff, &N_minus_1));
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
