#include "_file_.h"

void save_open_key(char* filename, number* mod, number* pubkey) 
{
	FILE* file;
	int i, j; // iterators 
	int c;
	file = fopen(filename, "w");

	if (file == NULL)
	{
		_log("Error while saving open key file");
		exit(FILE_OPEN_FAILURE);
	}
	fprintf(file, "_n_");

	while (((mod->current_count) - 1) % 4 != 0)
	{
		add_digit(mod, 0);
	}

	for (i = 0; i < mod->current_count - 1; i+=4)
	{
		c = 0;
		for (j = i+3; j >= i; j--)
		{
			c = c * 2 + mod->mas[j];
		}
		fprintf(file, "%c", (char)(c + (int)'a'));
	}

	fprintf(file, "#\n_e_");
	while (((pubkey->current_count) - 1) % 4 != 0)
	{
		add_digit(pubkey, 0);
	}

	for (i = 0; i < pubkey->current_count - 1; i += 4)
	{
		c = 0;
		for (j = i + 3; j >= i; j--)
		{
			c = c * 2 + pubkey->mas[j];
		}
		fprintf(file, "%c", (char)(c + (int)'a'));
	}
	fprintf(file, "#\n&");

	fclose(file);
}

void save_secret_key(char* filename, number* seckey)
{

}

number get_prime_from_file(char* filename, int line_number, int bit_size) 
{
	FILE* file; 
	int c = 0, line_num;
	char str[3];
	number res = init();

	file = fopen(filename, "r");
	if (file == NULL)
	{
		_log("Error while opening primes file");
		exit(FILE_OPEN_FAILURE);
	}
	while (c != (int)'&')
	{
		c = fgetc(file);
		if (c == (int)'_')
		{
			line_num = 0;
			c = fgetc(file);
			while (c != (int)'_')
			{
				line_num *= 10;
				line_num += (int)(c - '0');
				c = fgetc(file);
			}
			c = fgetc(file);
			if (line_num != line_number)
			{
				while (c != (int)'#')
				{
					c = fgetc(file);
				}
			}
			else
			{
				while (c != (int)'#')
				{
					add_digit(&res, (int)(c - '0'));
					c = fgetc(file);
				}
				if (res.current_count-1 != bit_size)
				{
					_log("Doesnt got prime number succesfully. Bit size doesnt match");
					fclose(file);
					exit(GET_PRIME_FAILURE);
				}
				_log("Got prime number succesfully");
				fclose(file);
				return res;
			}
		}
	}

	sprintf(str, "%d", line_number);
	_log("Doesnt got prime number succesfully. No prime. Line number was: ");
	_log(str);
	fclose(file);
	exit(GET_PRIME_FAILURE);
}