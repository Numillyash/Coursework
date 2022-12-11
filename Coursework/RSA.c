#include "RSA.h"

BOOL check_sign_file(char *input_filename, char *pubkey_filename, char *sign_filename)
{
	number n, e, msg, msg_ci;
	int c = 0;
	int c2 = 0;
	FILE *input, *sign;
	read_key(pubkey_filename, &n, &e, 'e');

	msg_ci = init();

	input = check_file_exist_read(input_filename);
	sign = check_file_exist_read(sign_filename);

	while (read_num_from_file(sign, &msg_ci))
	{
		msg = module_pow(&msg_ci, &e, &n);
		c = number_to_int(&msg) - 100;
		c2 = fgetc(input);
		if (c2 == EOF || c2 != c)
		{
			clear_mem(&msg);
			clear_mem(&msg_ci);
			clear_mem(&e);
			clear_mem(&n);

			fclose(input);
			fclose(sign);
			return 0;
		}
		clear_mem(&msg);
		clear_mem(&msg_ci);
		msg_ci = init();
	}

	clear_mem(&e);
	clear_mem(&n);
	clear_mem(&msg_ci);

	fclose(input);
	fclose(sign);
	return 1;
}

void signify_file(char *input_filename, char *seckey_filename, char *sign_filename)
{
	number n, d, msg, msg_ci;
	int c = 0;
	FILE *input, *sign;
	read_key(seckey_filename, &n, &d, 'd');

	input = check_file_exist_read(input_filename);
	sign = check_file_exist_write(sign_filename);

	while ((c = fgetc(input)) != EOF)
	{
		msg = int_to_number(c + 100);
		msg_ci = module_pow(&msg, &d, &n);
		save_num_to_file(sign, &msg_ci);
		clear_mem(&msg);
		clear_mem(&msg_ci);
	}

	fprintf(sign, "EOF");

	clear_mem(&d);
	clear_mem(&n);

	fclose(input);
	fclose(sign);
	_log("Signify complete");
}

void decrypt_file(char *input_filename, char *seckey_filename, char *output_filename)
{
	number n, d, msg, msg_ci;
	int c = 0;
	FILE *input, *output;
	read_key(seckey_filename, &n, &d, 'd');

	msg_ci = init();

	input = check_file_exist_read(input_filename);
	output = check_file_exist_write(output_filename);

	while (read_num_from_file(input, &msg_ci))
	{
		msg = module_pow(&msg_ci, &d, &n);
		c = number_to_int(&msg) - 100;
		fprintf(output, "%c", (char)c);
		clear_mem(&msg);
		clear_mem(&msg_ci);
		msg_ci = init();
	}

	// cleanup

	clear_mem(&d);
	clear_mem(&n);
	clear_mem(&msg_ci);

	fclose(input);
	fclose(output);
}

void encrypt_file(char *input_filename, char *pubkey_filename, char *output_filename)
{
	number n, e, msg, msg_ci;
	int c = 0;
	FILE *input, *output;
	read_key(pubkey_filename, &n, &e, 'e');

	input = check_file_exist_read(input_filename);
	output = check_file_exist_write(output_filename);

	while ((c = fgetc(input)) != EOF)
	{
		msg = int_to_number(c + 100);
		msg_ci = module_pow(&msg, &e, &n);
		save_num_to_file(output, &msg_ci);
		clear_mem(&msg);
		clear_mem(&msg_ci);
	}

	fprintf(output, "EOF");

	clear_mem(&e);
	clear_mem(&n);

	fclose(input);
	fclose(output);
	_log("Encrypt complete");
}

void generate_key(char *key_size_str, char *pubkey_filename, char *seckey_filename)
{
	KEY_BIT_SIZE keysize;
	int key_s_buf = 0, rand_num_p = -1, rand_num_q = -1;
	number p, q, n, phi_n, e, d, buff1, buff2, numb_one, numb_zero;
	number *arr;

	key_s_buf = atoi(key_size_str);
	if (key_s_buf == 0)
	{
		printf("Wrong key size: %s.\nType -h for help", key_size_str);
		_log("Error when try to make string \"key size\" to integer");
		exit(FAILURE);
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
			_log("Error: key size doesnt belong to keySizes list");
			exit(FAILURE);
			break;
		}
	srand((unsigned int)time(NULL));

	while (rand_num_p == rand_num_q)
	{
		rand_num_p = rand() % 500 + 1;
		rand_num_q = rand() % 500 + 1;
	}

	switch (keysize)
	{
	case KEY_256:
		p = get_prime_from_file(primes_128_bit_filename, rand_num_p, KEY_256 / 2);
		q = get_prime_from_file(primes_128_bit_filename, rand_num_q, KEY_256 / 2);
		break;
	case KEY_512:
		p = get_prime_from_file(primes_256_bit_filename, rand_num_p, KEY_512 / 2);
		q = get_prime_from_file(primes_256_bit_filename, rand_num_q, KEY_512 / 2);
		break;
	case KEY_1024:
		p = get_prime_from_file(primes_512_bit_filename, rand_num_p, KEY_1024 / 2);
		q = get_prime_from_file(primes_512_bit_filename, rand_num_q, KEY_1024 / 2);
		break;
	case KEY_2048:
		p = get_prime_from_file(primes_1024_bit_filename, rand_num_p, KEY_2048 / 2);
		q = get_prime_from_file(primes_1024_bit_filename, rand_num_q, KEY_2048 / 2);
		break;
	default:
		break;
	}

	_log("genkey: Got primes succesfully");

	numb_zero = int_to_number(0);
	numb_one = int_to_number(1);

	n = multiplication(&p, &q);

	_log("genkey: Got n succesfully");

	buff1 = difference(&p, &numb_one);
	buff2 = difference(&q, &numb_one);
	phi_n = multiplication(&buff1, &buff2);
	clear_mem(&buff1);
	clear_mem(&buff2);

	_log("genkey: Got phi_n succesfully");

	e = int_to_number(65537);
	_log("genkey: const done");

	arr = (number *)malloc(4 * sizeof(number)); // = { 1,0,0,1 };
	arr[0] = copy(&numb_one);					////////////////////////////////////////// !
	arr[1] = copy(&numb_zero);
	arr[2] = copy(&numb_zero);
	arr[3] = copy(&numb_one);
	_log("genkey: arr created");
	buff1 = euclide_algorithm_modifyed(&phi_n, &e, arr);
	clear_mem(&buff1);
	_log("genkey: euclide algorithm passed");
	d = copy(&arr[1]);

	if (d.mas[d.current_count - 1])
	{
		buff1 = addition(&d, &phi_n);
		clear_mem(&d);
		d = copy(&buff1);
		clear_mem(&buff1);
	}

	_log("genkey: all counts ready");

	clear_mem(&p);
	clear_mem(&q);
	clear_mem(&phi_n);
	clear_mem(&numb_one);
	clear_mem(&numb_zero);
	clear_mem(&arr[0]);
	clear_mem(&arr[1]);
	clear_mem(&arr[2]);
	clear_mem(&arr[3]);

	// File format
	// pubkey
	//_n_"smth"#
	//_e_"smth"#
	//&
	// seckey
	//_d_"smth"#
	//&
	// all numbers will be saved in 4-bit form
	_log("N");
	debug_log(&n);
	_log("E");
	debug_log(&e);
	_log("D");
	debug_log(&d);

	save_key(pubkey_filename, &n, &e, 'e');
	save_key(seckey_filename, &n, &d, 'd');

	clear_mem(&n);
	clear_mem(&e);
	clear_mem(&d);
	free(arr);
	exit(SUCCESS);
}
