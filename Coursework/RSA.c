#include "RSA.h"

number module_pow(number* a, number* t, number* b)
{
	number d, ost, iterator = init(), buff, buff2;
	number step = init(), st_2 = init();;

	add_digit(&step, 1);
	add_digit(&st_2, 2);
	add_digit(&iterator, 1);
	division_with_remainder(a, b, &d);
	ost = copy(&d);

	if (is_zero(&d))
	{
		clear_mem(&step);
		clear_mem(&st_2);
		clear_mem(&d);
		clear_mem(&iterator);
		return ost;
	}
	else
	{
		clear_mem(&iterator);
		iterator = copy(t);

		clear_mem(&ost);
		ost = copy(&step);

		buff2 = copy(a);
		while (!is_zero(&iterator))
		{
			if (iterator.mas[0] % 2 == 1)
			{
				buff = multiplication(&ost, &buff2);
				clear_mem(&ost);
				division_with_remainder(&buff, b, &ost);
				clear_mem(&buff);
			}
			buff = multiplication(&buff2, &buff2);
			clear_mem(&buff2);
			division_with_remainder(&buff, b, &buff2);
			clear_mem(&buff);

			buff = division(&iterator, &st_2);
			clear_mem(&iterator);
			iterator = copy(&buff);
			clear_mem(&buff);
		}
		buff = copy(&ost);
		clear_mem(&ost);
		division_with_remainder(&buff, b, &ost);
		clear_mem(&buff);
	}
	clear_mem(&d);
	clear_mem(&step);
	clear_mem(&iterator);
	return ost;
}

void generate_key(char* key_size_str, char* pubkey_filename, char* seckey_filename)
{
	FILE* pubkey, * seckey;
	KEY_BIT_SIZE keysize; int key_s_buf = 0;

	pubkey = fopen(pubkey_filename, "w");
	seckey = fopen(seckey_filename, "w");

	if (pubkey == NULL)
	{
		printf(FILE_OPENING_ERROR, pubkey_filename);
		exit(FILE_OPEN_ERROR);
	}
	if (seckey == NULL)
	{
		printf(FILE_OPENING_ERROR, seckey_filename);
		exit(FILE_OPEN_ERROR);
	}

	key_s_buf = atoi(key_size_str);
	if (key_s_buf == 0)
	{
		printf("Wrong key size: %s.\nType -h for help", key_size_str);
		exit(DEFAULT_ERROR);
	}
	else
		switch (key_s_buf)
		{
		case KEY_256:
			keysize = KEY_256;
			break;
		case KEY_512:
			keysize = KEY_512;
			break;
		case KEY_1024:
			keysize = KEY_1024;
			break;
		case KEY_2048:
			keysize = KEY_2048;
			break;
		default:
			printf("Wrong key size: %s.\nType -h for help", key_size_str);
			exit(DEFAULT_ERROR);
			break;
		}

	number p, q;



	fclose(pubkey); fclose(seckey);
}