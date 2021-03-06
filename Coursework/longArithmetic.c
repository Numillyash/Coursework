//
// Created by Georgul on 02.11.2021.
//
#include "longArithmetic.h"

number init() {
	number result = { 0, 1, 1 };
	result.mas = (unsigned short*)malloc(sizeof(unsigned short));
	return result;
}

number copy(number* value) {
	number result = init();
	for (int i = 0; i < value->current_count; i++)
		add_digit(&result, value->mas[i]);
	result.negative = value->negative;
	return result;
}

number normalize(number* value) {
	number result = init();
	int end = value->current_count - 1;
	for (int i = value->current_count - 1; i >= 0; i--) {
		end = i;
		if (value->mas[i] != 0) {
			break;
		}
	}
	for (int i = 0; i <= end; i++) {
		add_digit(&result, value->mas[i]);
	}
	result.negative = value->negative;
	return result;
}

number int_to_number(int value) {
	number result = init();
	if (value < 0)
	{
		result.negative = -1;
		value *= -1;
	}
	int ostatok;
	while (value > 0) {
		ostatok = value % KEYSIZE_MODULE;
		add_digit(&result, ostatok);
		value /= KEYSIZE_MODULE;
	}
	if (result.current_count == 0)
		add_digit(&result, 0);
	return result;
}

void clear_mem(number* value)
{
	free(value->mas);
	value->mas = NULL;
}

void add_digit(number* object, unsigned short value) {
	int iter = 0;
	unsigned short* buff;

	if (object->current_count < object->size) {
		object->mas[object->current_count] = value;
		object->current_count += 1;
	}
	else {
		buff = (unsigned short*)malloc(sizeof(unsigned short) * (object->size));
		for (iter = 0; iter < object->current_count; ++iter)
		{
			buff[iter] = object->mas[iter];
		}
		clear_mem(object);
		object->mas = (unsigned short*)malloc(sizeof(unsigned short) * (object->size) * 2);
		for (iter = 0; iter < object->current_count; ++iter)
		{
			object->mas[iter] = buff[iter];
		}
		free(buff);
		if (object->mas != NULL)
		{
			object->mas[object->current_count] = value;
		}
		object->current_count += 1;
		object->size *= 2;
	}
}

void offset_right(number* object)
{
	reverse(object);
	object->current_count -= 1;
	reverse(object);
	*object = normalize(object);
}

void offset_left(number* object)
{
	reverse(object);
	add_digit(object, 0);
	reverse(object);
	*object = normalize(object);
}

void reverse(number* value)
{
	number prom = init();
	for (int i = value->current_count - 1; i >= 0; i--)
	{
		add_digit(&prom, value->mas[i]);
	}
	for (int i = 0; i < prom.current_count; i++)
	{
		value->mas[i] = prom.mas[i];
	}
	clear_mem(&prom);
}

void print_number(number* value) {
	if (value->negative == -1)
		printf("-");
	for (int i = value->current_count - 1; i >= 0; i--)
	{
		int x = value->mas[i];
		printf("%d ", value->mas[i]);
	}
	printf("\n");
}

number addition(number* value1, number* value2) {
	number a, b;
	number result = init(), buff;
	char razr = 0;

	if (value1->negative == 1 && value2->negative == -1) {
		b = copy(value2);
		b.negative = 1;
		result = difference(value1, &b);
		return result;
	}
	else if (value1->negative == -1 && value2->negative == 1) {
		b = copy(value1);
		b.negative = 1;
		result = difference(value2, &b);
		return result;
	}
	else if (value1->negative == -1 && value2->negative == -1) {
		a = copy(value1);
		a.negative = 1;
		b = copy(value2);
		b.negative = 1;
		result = addition(&a, &b);
		result.negative = -1;
		return result;
	}

	if ((value1->current_count) >= (value2->current_count)) {
		a = copy(value1);
		b = copy(value2);
	}
	else {
		a = copy(value2);
		b = copy(value1);
	}

	for (int i = 0; i < a.current_count; i++) {
		if (i < b.current_count) {
			if (a.mas[i] + b.mas[i] + razr > (KEYSIZE_MODULE - 1)) {
				add_digit(&result, a.mas[i] + b.mas[i] + razr - KEYSIZE_MODULE);
				razr = 1;
			}
			else {
				add_digit(&result, a.mas[i] + b.mas[i] + razr);
				razr = 0;
			}
		}
		else {
			if (razr == 1) {
				if (a.mas[i] + razr > (KEYSIZE_MODULE - 1)) {
					add_digit(&result, a.mas[i] + razr - KEYSIZE_MODULE);
					razr = 1;
				}
				else {
					add_digit(&result, a.mas[i] + razr);
					razr = 0;
				}
			}
			else {
				add_digit(&result, a.mas[i]);
			}
		}
	}
	if (razr == 1) {
		add_digit(&result, razr);
		razr = 0;
	}

	buff = normalize(&result);
	clear_mem(&result);
	result = copy(&buff);
	clear_mem(&buff);

	clear_mem(&a);
	clear_mem(&b);

	return result;
}

number difference(number* value1, number* value2) {
	number a, b;
	number result = init(), buff;
	char razr = 0;

	if (value1->negative == 1 && value2->negative == -1) {
		b = copy(value2);
		b.negative = 1;
		result = addition(value1, &b);
		return result;
	}
	else if (value1->negative == -1 && value2->negative == 1) {
		b = copy(value1);
		b.negative = 1;
		result = addition(value2, &b);
		result.negative = -1;
		return result;
	}
	else if (value1->negative == -1 && value2->negative == -1) {
		a = copy(value1);
		a.negative = 1;
		b = copy(value2);
		b.negative = 1;
		result = difference(&b, &a);
		result.negative = 1;
		return result;
	}

	if ((value1->current_count) >= (value2->current_count)) {
		a = copy(value1);
		b = copy(value2);
	}
	else {
		result.negative *= -1;
		a = copy(value2);
		b = copy(value1);
	}

	for (int i = 0; i < a.current_count; i++) {
		if (i < b.current_count) {
			if (a.mas[i] - b.mas[i] - razr < 0) {
				add_digit(&result, a.mas[i] - b.mas[i] - razr + KEYSIZE_MODULE);
				razr = 1;
			}
			else {
				add_digit(&result, a.mas[i] - b.mas[i] - razr);
				razr = 0;
			}
		}
		else {
			if (razr == 1) {
				if (a.mas[i] - razr < 0) {
					add_digit(&result, a.mas[i] - razr + KEYSIZE_MODULE);
					razr = 1;
				}
				else {
					add_digit(&result, a.mas[i] - razr);
					razr = 0;
				}
			}
			else {
				add_digit(&result, a.mas[i]);
			}
		}
	}

	if (razr == 1) {
		number new = init();
		for (int i = 0; i < result.current_count; i++)
			add_digit(&new, 0);
		add_digit(&new, 1);
		result = difference(&new, &result);
		result.negative *= -1;
		razr = 0;
		clear_mem(&new);
	}

	buff = normalize(&result);
	clear_mem(&result);
	result = copy(&buff);
	clear_mem(&buff);

	clear_mem(&a);
	clear_mem(&b);

	return result;
}

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

		buff = normalize(&pr);
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
	if (value1->negative != value2->negative)
		return FALSE;
	if (value1->current_count != value2->current_count)
		return FALSE;
	for (iterator = 0; iterator < value1->current_count; iterator++)
	{
		if (value1->mas[iterator] != value2->mas[iterator])
			return FALSE;
	}

	return TRUE;
}