#include "RSA.h"

void generate_key(char* key_size_str, char* pubkey_filename, char* seckey_filename)
{
	FILE* pubkey, * seckey;
	KEY_BIT_SIZE keysize; 
	int key_s_buf = 0, rand_num_p = -1, rand_num_q = -1;
	number p, q, n, phi_n, e, d, buff1, buff2, numb_one, numb_zero;
	number* arr;

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

	while (rand_num_p == rand_num_q) {
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


	numb_zero = int_to_number(0);
	numb_one = int_to_number(1);

	n = multiplication(&p, &q);

	buff1 = difference(&p, &numb_one);
	buff2 = difference(&q, &numb_one);
	phi_n = multiplication(&buff1, &buff2);
	clear_mem(&buff1);
	clear_mem(&buff2);

	e = int_to_number(65537);

	arr = (number*)malloc(4 * sizeof(number)); // = { 1,0,0,1 };
	arr[0] = copy(&numb_one);
	arr[1] = copy(&numb_zero);
	arr[2] = copy(&numb_zero);
	arr[3] = copy(&numb_one);
	buff1 = euclide_algorithm_modifyed(&phi_n, &e, arr);
	clear_mem(&buff1);
	d = copy(&arr[1]);

	clear_mem(&p);
	clear_mem(&q);
	clear_mem(&phi_n);
	clear_mem(&numb_one);
	clear_mem(&numb_zero);
	clear_mem(&arr[0]);
	clear_mem(&arr[1]);
	clear_mem(&arr[2]);
	clear_mem(&arr[3]);

	//File format
	//pubkey
	//_n_"smth"#
	//_e_"smth"#
	//&
	//seckey
	//_d_"smth"#
	//&
	//all numbers will be saved in 4-bit form

	save_open_key(pubkey_filename, &n, &e);
	save_secret_key(seckey_filename, &d);

	clear_mem(&n);
	clear_mem(&e);
	clear_mem(&d);

	exit(DEBUG_EXIT_CODE);
}

void signify_file(char* input_filename, char* sec_code_filename, char* output_filename) {}
