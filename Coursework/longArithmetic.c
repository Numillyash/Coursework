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
		add_element(&result, value->mas[i]);
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
		add_element(&result, value->mas[i]);
	}
	result.negative = value->negative;
	return result;
}

void reverse(number* value)
{
	number prom = init();
	for (int i = value->current_count - 1; i >= 0; i--)
	{
		add_element(&prom, value->mas[i]);
	}
	for (int i = 0; i < prom.current_count; i++)
	{
		value->mas[i] = prom.mas[i];
	}
	free(prom.mas);
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

void add_element(number* object, unsigned short value) {
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
		free(object->mas);
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
	//
}

void remove_last_digit(number* object)
{
	reverse(object);
	object->current_count -= 1;
	reverse(object);
	*object = normalize(object);
}

number plus(number* value1, number* value2) {
	number a, b;
	number result = init(), buff;
	char razr = 0;

	if (value1->negative == 1 && value2->negative == -1) {
		b = copy(value2);
		b.negative = 1;
		result = minus(value1, &b);
		return result;
	}
	else if (value1->negative == -1 && value2->negative == 1) {
		b = copy(value1);
		b.negative = 1;
		result = minus(value2, &b);
		return result;
	}
	else if (value1->negative == -1 && value2->negative == -1) {
		a = copy(value1);
		a.negative = 1;
		b = copy(value2);
		b.negative = 1;
		result = plus(&a, &b);
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
				add_element(&result, a.mas[i] + b.mas[i] + razr - KEYSIZE_MODULE);
				razr = 1;
			}
			else {
				add_element(&result, a.mas[i] + b.mas[i] + razr);
				razr = 0;
			}
		}
		else {
			if (razr == 1) {
				if (a.mas[i] + razr > (KEYSIZE_MODULE - 1)) {
					add_element(&result, a.mas[i] + razr - KEYSIZE_MODULE);
					razr = 1;
				}
				else {
					add_element(&result, a.mas[i] + razr);
					razr = 0;
				}
			}
			else {
				add_element(&result, a.mas[i]);
			}
		}
	}
	if (razr == 1) {
		add_element(&result, razr);
		razr = 0;
	}

	buff = normalize(&result);
	free(result.mas);
	result = copy(&buff);
	free(buff.mas);

	free(a.mas); a.mas = NULL;
	free(b.mas); b.mas = NULL;

	return result;
}

number minus(number* value1, number* value2) {
	number a, b;
	number result = init(), buff;
	char razr = 0;

	if (value1->negative == 1 && value2->negative == -1) {
		b = copy(value2);
		b.negative = 1;
		result = plus(value1, &b);
		return result;
	}
	else if (value1->negative == -1 && value2->negative == 1) {
		b = copy(value1);
		b.negative = 1;
		result = plus(value2, &b);
		result.negative = -1;
		return result;
	}
	else if (value1->negative == -1 && value2->negative == -1) {
		a = copy(value1);
		a.negative = 1;
		b = copy(value2);
		b.negative = 1;
		result = minus(&b, &a);
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
				add_element(&result, a.mas[i] - b.mas[i] - razr + KEYSIZE_MODULE);
				razr = 1;
			}
			else {
				add_element(&result, a.mas[i] - b.mas[i] - razr);
				razr = 0;
			}
		}
		else {
			if (razr == 1) {
				if (a.mas[i] - razr < 0) {
					add_element(&result, a.mas[i] - razr + KEYSIZE_MODULE);
					razr = 1;
				}
				else {
					add_element(&result, a.mas[i] - razr);
					razr = 0;
				}
			}
			else {
				add_element(&result, a.mas[i]);
			}
		}
	}

	if (razr == 1) {
		number new = init();
		for (int i = 0; i < result.current_count; i++)
			add_element(&new, 0);
		add_element(&new, 1);
		result = minus(&new, &result);
		result.negative *= -1;
		razr = 0;
		free(new.mas);
	}

	buff = normalize(&result);
	free(result.mas);
	result = copy(&buff);
	free(buff.mas);

	free(a.mas); a.mas = NULL;
	free(b.mas); b.mas = NULL;

	return result;
}

number proizv_to_digit(number* value1, int value2) {
	number sum = init();
	int count = 0;
	number pr = init(), res = init(), buff;
	int prom, i, j;

	for (i = 0; i < value1->current_count; i++)
	{
		prom = (int)(value1->mas[i]);
		prom *= value2;

		buff = int_to_number(prom);
		free(pr.mas);
		pr = copy(&buff);
		free(buff.mas);

		buff = normalize(&pr);
		free(pr.mas);
		pr = copy(&buff);
		free(buff.mas);

		buff = init();
		free(res.mas);
		res = copy(&buff);
		free(buff.mas);

		for (j = 0; j < count; ++j) {
			add_element(&res, 0);
		}
		for (j = 0; j < pr.current_count; ++j) {
			add_element(&res, (int)pr.mas[j]);
		}

		buff = plus(&sum, &res);
		free(sum.mas); 
		sum.mas = NULL;
		sum = copy(&buff);
		free(buff.mas);
		buff.mas = NULL;

		++count;
	}

	free(pr.mas); pr.mas = NULL;
	free(res.mas); res.mas = NULL;

	return sum;
}

number proizv(number* value1, number* value2) {
	number sum = init();
	int count = 0;
	for (int i = 0; i < value2->current_count; i++)
	{
		int pr = (int)(value2->mas[i]);
		number prom = proizv_to_digit(value1, pr);

		number res = init();
		for (int j = 0; j < count; ++j) {
			add_element(&res, 0);
		}
		for (int j = 0; j < prom.current_count; ++j) {
			add_element(&res, (int)prom.mas[j]);
		}
		sum = plus(&sum, &res);
		++count;

		free(prom.mas); prom.mas = NULL;
		free(res.mas); res.mas = NULL;
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

number int_to_number(int value) {
	number result = init();
	int ostatok;
	while (value > 0) {
		ostatok = value % KEYSIZE_MODULE;
		add_element(&result, ostatok);
		value /= KEYSIZE_MODULE;
	}
	if (result.current_count == 0)
		add_element(&result, 0);
	return result;
}

number degree(number* value1, number* value2)
{
	number result = init();
	add_element(&result, 1);
	number prom = init();
	add_element(&prom, 1);
	number iter = copy(value2);

	while (!(iter.current_count == 1 && iter.mas[0] == 0))
	{
		iter = minus(&iter, &prom);
		normalize(&iter);
		result = proizv(&result, value1);
	}
	return result;
}

number deli_v_stolbik(number* value1, number* value2, number* ost)
{
	number osn = int_to_number(256);
	number quot = int_to_number(0), rem = copy(value1), sub = copy(value2);
	number add = int_to_number(1);
	number buff;
	short shifts = 1;

	long long counter = 0;

	*ost = init();

	while ((buff = minus(&sub, &rem)).negative == -1)
	{
		free(buff.mas); buff.mas = NULL;
		buff = proizv(&sub, &osn);
		free(sub.mas); sub.mas = NULL;
		sub = copy(&buff);

		free(buff.mas); buff.mas = NULL;
		buff = proizv(&add, &osn);
		free(add.mas); add.mas = NULL;
		add = copy(&buff);
		free(buff.mas); buff.mas = NULL;
		shifts++;
	}
	counter = (long long)shifts - 1;
	free(buff.mas); buff.mas = NULL;
	while (shifts)
	{
		while ((buff = minus(&rem, &sub)).negative != -1)
		{
			free(buff.mas); buff.mas = NULL;
			buff = minus(&rem, &sub);
			free(rem.mas); rem.mas = NULL;
			rem = copy(&buff);
			free(buff.mas); buff.mas = NULL;

			buff = plus(&add, &quot);
			free(quot.mas); quot.mas = NULL;
			quot = copy(&buff);
			free(buff.mas); buff.mas = NULL;
			counter++;
		}
		remove_last_digit(&sub);
		remove_last_digit(&add);
		shifts--;
	}

	printf("Count = %lld\n", counter);

	free(osn.mas); osn.mas = NULL;
	free(sub.mas); sub.mas = NULL;
	free(add.mas); add.mas = NULL;

	*ost = rem;
	return quot;
}