#include "RSA.h"

void generate_key(char* key_size_str, char* pubkey_filename, char* seckey_filename)
{
	FILE* pubkey, * seckey;
	KEY_BIT_SIZE keysize; int key_s_buf = 0;

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

	number p, q;

	fclose(pubkey); fclose(seckey);
}
