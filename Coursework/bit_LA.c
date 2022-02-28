//
// Created by Georgul on 02.11.2021.
//
#include "bit_LA.h"

uint8_t XOR(uint8_t a, uint8_t b)
{
	return a ^ b;
}

uint8_t NOT(uint8_t a)
{
	return (a) ? 0 : 1;
}

uint8_t AND(uint8_t a, uint8_t b)
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
	int iter = 0;
	uint8_t* buff;

	if (object->current_count < object->size) {
		object->mas[object->current_count] = object->mas[object->current_count - 1];
		object->mas[object->current_count - 1] = value;
		object->current_count += 1;
	}
	else {
		buff = (uint8_t*)malloc(sizeof(uint8_t) * (object->size));
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
	for (int i = value->current_count - 1; i >= 0; i--)
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

/*
number mult_to_digit(number* value1, int value2) {
	number sum = init();
	int count = 0;
	number pr = init(), res = init(), buff;
	int prom, i, j;

	for (i = 0; i < value1->current_count; i++)
	{
		prom = (int)(value1->mas[i]);
		prom *= value2;

		buff = int_to_number(prom);
		clear_mem(&pr);
		pr = copy(&buff);
		clear_mem(&buff);

		normalize(&pr);
		buff = copy(&pr);
		clear_mem(&pr);
		pr = copy(&buff);
		clear_mem(&buff);

		buff = init();
		clear_mem(&res);
		res = copy(&buff);
		clear_mem(&buff);

		for (j = 0; j < count; ++j) {
			add_digit(&res, 0);
		}
		for (j = 0; j < pr.current_count; ++j) {
			add_digit(&res, (int)pr.mas[j]);
		}

		buff = addition(&sum, &res);
		clear_mem(&sum);
		sum = copy(&buff);
		clear_mem(&buff);

		++count;
	}

	clear_mem(&pr);
	clear_mem(&res);

	return sum;
}

number multiplication(number* value1, number* value2) {
	number sum = init();
	int count = 0;
	for (int i = 0; i < value2->current_count; i++)
	{
		int pr = (int)(value2->mas[i]);
		number prom = mult_to_digit(value1, pr);

		number res = init();
		for (int j = 0; j < count; ++j) {
			add_digit(&res, 0);
		}
		for (int j = 0; j < prom.current_count; ++j) {
			add_digit(&res, (int)prom.mas[j]);
		}
		sum = addition(&sum, &res);
		++count;

		clear_mem(&prom);
		clear_mem(&res);
	}
	if (value1->negative == 1 && value2->negative == 1) {
		return sum;
	}
	else if (value1->negative == 1 && value2->negative == -1) {
		sum.negative = -1;
		return sum;
	}
	else if (value1->negative == -1 && value2->negative == 1) {
		sum.negative = -1;
		return sum;
	}
	else if (value1->negative == -1 && value2->negative == -1) {
		return sum;
	}
}

number degree(number* value1, number* value2)
{
	number result = init();
	add_digit(&result, 1);
	number prom = init();
	add_digit(&prom, 1);
	number iter = copy(value2);

	while (!(iter.current_count == 1 && iter.mas[0] == 0))
	{
		iter = difference(&iter, &prom);
		normalize(&iter);
		result = multiplication(&result, value1);
	}
	return result;
}

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

number division_with_remainder(number* value1, number* value2, number* ost)
{
	number osn = int_to_number(256);
	number quot = int_to_number(0), rem = copy(value1), sub = copy(value2);
	number add = int_to_number(1);
	number buff;
	short shifts = 1;

	if (value1->negative == -1)
	{
		// b - ( |a| % b )
		buff = copy(value1);
		buff.negative = 1;
		number _ost;
		number result = division_with_remainder(&buff, value2, &_ost); // ost = ( |a| % b )
		clear_mem(&buff);

		buff = difference(value2, &_ost);
		clear_mem(&_ost);
		_ost = copy(&buff);
		clear_mem(&buff);

		buff = addition(&result, &add);
		clear_mem(&result);
		result = copy(&buff);
		clear_mem(&buff);

		*ost = _ost;
		result.negative = -1;
		clear_mem(&osn);
		clear_mem(&sub);
		clear_mem(&add);
		clear_mem(&rem);
		clear_mem(&quot);
		return result;
	}



	*ost = init();

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

	*ost = rem;
	return quot;
}


*/