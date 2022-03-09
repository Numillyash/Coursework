#include "RSA.h"

void generate_key(char* key_size_str, char* pubkey_filename, char* seckey_filename)
{
	FILE* pubkey, * seckey;
	KEY_BIT_SIZE keysize; 
	int key_s_buf = 0, rand_num_p = -1, rand_num_q = -1;
	number p, q;

	pubkey = fopen(pubkey_filename, "w");
	seckey = fopen(seckey_filename, "w");

	if (pubkey == NULL)
	{
		printf(FILE_OPENING_ERROR, pubkey_filename);
		_log("Error when try to open \"pubkey\" file");
		exit(FILE_OPEN_FAILURE);
	}
	if (seckey == NULL)
	{
		printf(FILE_OPENING_ERROR, seckey_filename);
		_log("Error when try to open \"seckey\" file");
		exit(FILE_OPEN_FAILURE);
	}

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

	print_number(&p);
	print_number(&q);

	fclose(pubkey); fclose(seckey);
	exit(DEBUG_EXIT_CODE);
}

void signify_file(char* input_filename, char* sec_code_filename, char* output_filename) {}
