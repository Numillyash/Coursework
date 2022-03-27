#include "_file_.h"

FILE* check_file_exist_write(char* filename)
{
	FILE* file;
	file = fopen(filename, "w");

	if (file == NULL)
	{
		_log("Error while saving open key file");
		exit(FILE_OPEN_FAILURE);
	}
	return file;
}

FILE* check_file_exist_read(char* filename)
{
	FILE* file;
	file = fopen(filename, "r");

	if (file == NULL)
	{
		_log("Error while saving open key file");
		exit(FILE_OPEN_FAILURE);
	}
	return file;
}

//TODO: запихнуть чтение из файла ключа как юнион
// »де€
// символ читаетс€ как число в 4 бита
// тогда из двоичного числа можно получать сразу биты, мину€ перевод в 2сс
//
void save_open_key(char* filename, number* mod, number* pubkey) 
{
	FILE* file;
	int i, j; // iterators 
	int c;
	file = check_file_exist_write(filename);

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

	_log("Open key saved succesfully");
}

void save_secret_key(char* filename, number* seckey)
{
	FILE* file;
	int i, j; // iterators 
	int c;
	file = check_file_exist_write(filename);
	fprintf(file, "_d_");

	while (((seckey->current_count) - 1) % 4 != 0)
	{
		add_digit(seckey, 0);
	}

	for (i = 0; i < seckey->current_count - 1; i += 4)
	{
		c = 0;
		for (j = i + 3; j >= i; j--)
		{
			c = c * 2 + seckey->mas[j];
		}
		fprintf(file, "%c", (char)(c + (int)'a'));
	}
	fprintf(file, "#\n&");

	fclose(file);

	_log("Secret key saved succesfully");
}

void read_open_key(char* filename, number* mod, number* pubkey)
{
	FILE* file;
	int i, j; // iterators 
	int c;
	int ms[4];
	
	*mod = init(); *pubkey=init();
	file = check_file_exist_read(filename);

	char buff[2048];

	fgets(buff, 2048, file);

	i = 3;
	while ((c = (int)buff[i]) != (int)'#')
	{
		c = (int)(buff[i] - 'a');
		for (j = 0; j < 4; j++)
		{
			ms[j] = c % 2;
			c >>= 1;
		}
		for (j = 0; j < 4; j++)
		{
			add_digit(mod, ms[j]);
		}
		i++;
	}

	fgets(buff, 2048, file);

	i = 3;
	while ((c = (int)buff[i]) != (int)'#')
	{
		c = (int)(buff[i] - 'a');
		for (j = 0; j < 4; j++)
		{
			ms[j] = c % 2;
			c >>= 1;
		}
		for (j = 0; j < 4; j++)
		{
			add_digit(pubkey, ms[j]);
		}
		i++;
	}

	normalize(mod);
	normalize(pubkey);
	fclose(file);
	_log("Open key parced succesfully");
}

void read_secret_key(char* filename, number* seckey)
{
	FILE* file;
	int i, j; // iterators 
	int c;
	int ms[4];

	*seckey = init();
	file = check_file_exist_read(filename);

	char buff[2048];

	fgets(buff, 2048, file);

	i = 3;
	while ((c = (int)buff[i]) != (int)'#')
	{
		c = (int)(buff[i] - 'a');
		for (j = 0; j < 4; j++)
		{
			ms[j] = c % 2;
			c >>= 1;
		}
		for (j = 0; j < 4; j++)
		{
			add_digit(seckey, ms[j]);
		}
		i++;
	}

	normalize(seckey);
	fclose(file);
	_log("Secret key parced succesfully");
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